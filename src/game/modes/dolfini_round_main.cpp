// Dolfini round (prize game): CRoundDolMain's state screens with the 3D scene, the gallery
// and prize images, and the finish spin and dolphin animations.
// It also holds the weighted reward selection that applies prizes to the player's inventory.

extern "C" char s_dolfiniReloadingDtorRegistration[];
extern "C" char s_roundDolDrawBpDestructorRecord[];
// Registration records occupy 12 bytes each; the final buffer holds a temporary resource filename.
struct DolfiniModuleStorage {
    char inputDestructorRecord[12];
    char bpDestructorRecord[12];
    char bpHandleDestructorRecord[12];
    char reserved24[12];
    char fadeDestructorRecord[12];
    char sharedFileDestructorRecord[12];
    char reserved48[12];
    char loadingDestructorRecord[12];
    char messageFilename[1]; // address-only view of the original static scratch buffer
};
struct DolfiniDialogPool {
    unsigned short noTickets[0x3C / 2];
    unsigned short confirmExit[0x34 / 2];
    unsigned short prizeMessagePrefix[1];
};
#include "iff/IffStruct.h"
// Address-only layout of the existing string pool; no new string storage.
struct DolfiniPrizeStringPool {
    char rewardTableFile[9];         // 0x0
    char galleryImageTableFile[18];  // 0x9
    char prizeGalleryTableFile[18];  // 0x1B
    char iff_msg_eky[12];            // 0x2D
    char dlh_title_b2d[14];          // 0x39
    char dlh_ingame_02_ARO_b2d[22];  // 0x47
    char dlh_ingame_01_COS_b2d[22];  // 0x5D
    char dlh_ingame_00_MSG_b2d[22];  // 0x73
    char dlh_result_b2d[15];         // 0x89
    char dlh_eps_b2d[12];            // 0x98
    char dlh_hukidasi_b2d[17];       // 0xA4
    char dolph_tojyo_b2d[16];        // 0xB5
    char dlh_result_spd[15];         // 0xC5
    char dlh_title_waku_IN[18];      // 0xD4
    char lensflare[10];              // 0xE6
    char gall_num_gsr[13];           // 0xF0
    char dlh_title_waku_MOV[19];     // 0xFD
    char spinning_dolph_wav[19];     // 0x110
    char party_character_decide[23]; // 0x123
    char dlh_title_waku_OUT[19];     // 0x13A
    char dlh_arrow_IN[13];           // 0x14D
    char dlh_selects_IN[15];         // 0x15A
    char dlh_MSG_01_IN[14];          // 0x169
    char dlh_selects_MOV[16];        // 0x177
    char dlf_MSG_01_MOV[15];         // 0x187
    char dlh_selects_OUT[16];        // 0x196
    char dlh_MSG_01_OUT[15];         // 0x1A6
    char dlf_MSG_02_MOV[15];         // 0x1B5
    char dlh_eps_A[10];              // 0x1C4
    char dlh_hukidasi_MOV[17];       // 0x1CE
    char dlh_eps_B[10];              // 0x1DF
    char dlh_eps_C[10];              // 0x1E9
    char dlh_eps_D[10];              // 0x1F3
    char dlh_eps_E[10];              // 0x1FD
    char dlh_MSG_02_OUT[15];         // 0x207
    char dlh_hukidasi_OUT[17];       // 0x216
    char zeroBytes_227[1];           // 0x227
    char dlh_gsr[9];                 // 0x228
    char tmb_gsr[9];                 // 0x231
    char dlh_res_item_00_gsr[20];    // 0x23A
    char dlh_result_IN[14];          // 0x24E
    char dlh_result_MOV[15];         // 0x25C
    char dlh_result_OUT[15];         // 0x26B
    char rarePrizeSound[19];         // 0x27A
    char cookiePrizeSound[23];       // 0x28D
};

#include "iff/IffStruct.h"
#include "game/chatmsg_match.h"

// Transition directory (each label below handles one tick, then breaks):
// WarmAnimations -> WaitResources -> TitleEnter -> TitleIdle -> TitleLeave;
// TitleLeave -> ArrowEnter -> ControlsEnter -> WaitSpinInput;
// WaitSpinInput -> ControlsLeave -> WaitSpinStart -> Spinning;
// Spinning -> ControlsEnter on insufficient speed, otherwise FinishSpin;
// FinishSpin -> PrizeEnter -> ChooseReplayOrExit;
// replay -> PrizeLeaveForReplay -> WaitSpinInput; exit -> SaveAfterPrize -> done.
// Cancel from controls -> ConfirmExit -> ControlsEnter or SaveAfterCancel -> done.
// Title without tickets -> NoTicketsDialog -> done. Inactive does no work.
// PrizeLeaveForExit only waits for the result script; no assignment to it occurs
// in this unit. Do not merge it with the save path based on its enum name alone.
// Session flow: warm/load (0/1) -> title (2..4) -> spin controls (5..9) ->
// spin/finish (10/11) -> prize/replay (12..14), or exit/save (15..18).
// State 19 is idle; 20 displays the no-ticket dialog. Update returns 1 only
// when this session can close; drawing uses these same states without advancing them.
// UI IDs 2/3 = replay/exit, 4 = item image, 5 = prize text, 6 = gallery
// image, 7 = no-ticket warning. Numeric entries 25/30 show tickets/prize quantity.
// Values are persisted in the session; transitions and drawing use the same states.
enum DolfiniState {
    kDolfiniWarmAnimations = 0,
    kDolfiniWaitResources = 1,
    kDolfiniTitleEnter = 2,
    kDolfiniTitleIdle = 3,
    kDolfiniTitleLeave = 4,
    kDolfiniArrowEnter = 5,
    kDolfiniControlsEnter = 6,
    kDolfiniWaitSpinInput = 7,
    kDolfiniControlsLeave = 8,
    kDolfiniWaitSpinStart = 9,
    kDolfiniSpinning = 10,
    kDolfiniFinishSpin = 11,
    kDolfiniPrizeEnter = 12,
    kDolfiniChooseReplayOrExit = 13,
    kDolfiniPrizeLeaveForReplay = 14,
    kDolfiniPrizeLeaveForExit = 15,
    kDolfiniConfirmExit = 16,
    kDolfiniSaveAfterPrize = 17,
    kDolfiniSaveAfterCancel = 18,
    kDolfiniInactive = 19,
    kDolfiniNoTicketsDialog = 20
};
enum {
    kDolfiniTicketItemId = 0x1A0000FE,
    kDolfiniItemLimit = 999,
    kDolfiniAuxPartKind = 0x1C,
    kDolfiniConfirmAction = 4,
    kDolfiniCancelAction = 6,
    kDolfiniSpinAction = 15,
    kDolfiniReplayElement = 2,
    kDolfiniExitElement = 3,
    kDolfiniPrizeImageElement = 4,
    kDolfiniPrizeTextElement = 5,
    kDolfiniGalleryImageElement = 6,
    kDolfiniNoTicketsElement = 7,
    kDolfiniTicketNumberEntry = 25,
    kDolfiniPrizeNumberEntry = 30,
    kDolfiniTitleGroup = 0x3F0,
    kDolfiniPrizeTextColor = 0x764717FF,
    kDolfiniFadeFrames = 15,
    kDolfiniInventoryReward = 0,
    kDolfiniRarePercentBase = 100,
    kDolfiniRandomDraws = 10,
    kDolfiniEquippedItemSlots = 8,
    kDolfiniTextureVertexFormat = 0x11F005,
    kDolfiniTriangleStrip = 0x98,
    kDolfiniQuadVertices = 4,
    kDolfiniQuadVertexBytes = 0x18
};
struct DolfiniInventoryView {
    char padding[0x174];
    short itemCounts[13];
    unsigned char auxiliaryParts[1]; // address-only start of the saved ownership flags
};

#include "classes/PlayerRecordOwner8030E168.h"
#define AsPlayerRecordOwner8030E168(p) ((PlayerRecordOwner8030E168*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "classes/CGuruguru.h"
#include "classes/CPangyaCommonDialog.h"
#include "classes/CRoundDolMain.h"
#include "classes/CRoundDol3D.h"
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CGallery.h"
#include "classes/CPrize.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CNowReloading.h"
#include "classes/CBpCtrl.h"
#include "classes/CProjectG.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CFontNumDraw.h"
#include "classes/CMsgDraw.h"

struct fn_8030EE64_String {
    unsigned int flags;
    unsigned int unused;
    char* text;
};

extern "C" {

/* ---- typed declarations for CRoundDolMain_UpdateStateMachine and the rest of the unit ---- */
void CBpCtrl_dtor(void*);
void* __register_global_object(void*, void*, void*);
void* __nw__FUl(unsigned int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void SharedFilePointerVector_InitReserved1024(void*);
void SharedFilePointerVector_Destroy(void*);
void NString_AssignCString(void*, const char*);
void appFreeUni__FPv(void*);
void __dt__13CNowReloadingFv(void*);
float SoundValuePair_GetSelectedValue(void*, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void SoundManager_SetBufferVolume(void*, float, float);
int Layout_PollLoadComplete(void*);
int FileList_GetLoadedBufferByName(void*, const char*); // Retained name: returns a borrowed loaded-buffer address.
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void FadeState_StartFadeOut(void*, int);
int LoadingIndicator_IsAbsentOrState3(void*);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetOpacity(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void UITextTable_SetEntryResourceName(void*, int, const char*);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void Layout_SetElementEnabled(void*, int, int);
void Sound_SetGurudolRoundVolume(void*);
int BpObject_IsCurrentScriptComplete(void*);
int Input_IsActionPressed(void*, int, int);
int ItemInventory_FindItemIndex(void*, int);
void* __ct__19CPangyaCommonDialogFPvi(void*, int, int);
int Input_IsActionButtonDown(void*, int, int);
void BpTextureBinding_ReleaseSources(void*);
void Text_SetPosition(void*, float*);
int wcslen(const char*);
void WString_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void* MessageTable_FindTextByName(void*, void*);
void* TextResource_FindNamedEntry(void*, void*);
void Text_SetContent(void*, const char*);
int PrizeGallery_FindMessageRecordOffset(void*, int); // PrizeGallery_FindMessageRecordOffset.
int PrizeGallery_GetImageId(void*, int);              // PrizeGallery_GetImageId.
int SharedFileTable_GetMappedValueAt(void*, int);     // GalleryImage_FindRecordOffset.
void* Table_ResolveDataOffset(void*, int);
void* OffsetStringNode_GetText(void*);
void* ItemInventory_FindDataWithHairStyleFallback(void*, int);
void* ItemInventory_FindDataById(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void* String_ConcatCStringAndString(void*, const char*, void*);
void String_ConstructConcatCStr(void*, void*, const char*);
int strlen(const char*);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_SetAlternateDrawEnabled(void*, int);
void FadeState_StartFadeIn(void*, int);
void Layout_Update(void*);
void Layout_CollectActivated(void*, void*, int);
void SharedFileList_CreateAndAppend(void*, const char*, int);
void SoundManager_RequestBgmStop(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void InputManager_SuspendCursorDrawing(void*);
void InputManager_ResumeCursorDrawing(void*);
void SaveData_Ctor(void*);
void SaveData_SetCurrentRecord(void*, void*);
void UIRender_SetDefaultState(void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_GetElementPosition(void*, int, float*);
void Layout_GetElementSize(void*, int, float*);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int MPBase1_Process(void*, float);
void Engine_SetFlexibleVertexFormat(int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, float*, int);
void MPBase1_DrawIfInitialized(void*);
void* memcpy(void*, const void*, unsigned int);
char* strcpy(char*, const char*);

extern signed char s_bpCtrlInitialized;
extern signed char s_sharedFileListInitialized;
extern void* g_chatMsg;
extern void* g_soundManager;
extern signed char s_fadeOverlayInitialized;
extern signed char s_nowReloadingInitialized;
extern unsigned char s_nowReloading[4];
extern void* g_project;
extern char s_fadeOverlay[];
extern char roundDolPrizeNumericEntries[];
extern char __vt__9CGuruguru[];
extern char __vt__14CGallery_image[];
extern char __vt__14CPrize_gallery[];
extern char roundDolDialogStrings[];
extern signed char s_inputManagerInitialized;
extern char s_roundDolInputDestructorRecord[];
extern char s_bpCtrl[];
extern char s_sharedFileListStorage[];
extern char s_gameInputManager[];
extern char s_saveData[];
extern char roundDolLayoutElements[]; // roundDolLayoutElements.
extern char roundDolPrizeStrings[];
extern char roundDolGalleryQuadTemplate[];      // roundDolGalleryQuadTemplate.
extern char roundDolPrizeQuadTemplate[];        // roundDolPrizeQuadTemplate.
extern char s_roundDolDrawBpDestructorRecord[]; // s_roundDolDrawBpDestructorRecord.
extern CSharedDoc* g_sharedDoc;
extern signed char s_saveDataInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern char s_bpCtrlHandle[8];
extern signed char s_bpCtrlInitialized;
extern int kRoundDolItemTextInitialXBits;    // kRoundDolItemTextInitialXBits.
extern int kRoundDolItemTextInitialYBits;    // kRoundDolItemTextInitialYBits.
extern int kRoundDolGalleryTextInitialXBits; // kRoundDolGalleryTextInitialXBits.
extern int kRoundDolGalleryTextInitialYBits; // kRoundDolGalleryTextInitialYBits.
extern float kFloat_0_RoundDol;
extern float kFloat_0_5_RoundDol;
extern float kFloat_200_RoundMenu;
extern float kFloat_1_RoundDolSound;
extern float kFloat_0_033_RoundDolMain;
void CBpCtrlHandle_Dtor(void*);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void BpObject_SetEnabled(void*, int);
int Dolfini_SelectAndApplyWeightedReward(struct DolfiniRewardState*);
struct DolfiniRewardState;

extern float s_roundDolSavedBufferVolumeStorage;
extern float kFloat_0_5_RoundDol;
extern float kFloat_200_RoundMenu;
extern float kFloat_1_RoundDolSound;

struct fn_8030F460_Coll {
    void* data;
    unsigned int size;
    int cap;
    // Initialize an empty owned activation-ID vector.
    fn_8030F460_Coll() : data(0), size(0), cap(0) {}
    // Free collected activation storage on every exit path.
    ~fn_8030F460_Coll() {
        if (data) {
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};

struct fn_8030F460_Allocator {
    char value;
};

struct fn_8030F460_Dialog {
    char storage[0x44];
    // Construct the owned common dialog with its original kind/count arguments.
    fn_8030F460_Dialog(int kind, int count) {
        __ct__19CPangyaCommonDialogFPvi(this, kind, count);
    }
};

struct fn_8030F460_String {
    unsigned int flags;
    unsigned int length;
    char* text;
    // Initialize an empty inline string.
    fn_8030F460_String() : flags(0), length(0), text(0) {}
    // Leave storage for the concat routines that construct the string in place.
    fn_8030F460_String(int) {}
    // Release the string only when the high flag bit denotes heap ownership.
    ~fn_8030F460_String() {
        if (flags >> 31)
            appFreeUni__FPv(text);
    }
    // Read the inline or heap string length (wide-string callers use code units).
    unsigned int size() const {
        return !(flags >> 31) ? (*(const unsigned char*)this & 0x7F) : length;
    }
    // Borrow the current narrow-string character storage.
    char* data() {
        return !(flags >> 31) ? (char*)this + 1 : text;
    }
    // Copy inline words or replace the current content with a deep string copy.
    void assign(fn_8030F460_String& other) {
        unsigned int isLong = flags >> 31;
        if (!isLong && !(other.flags >> 31)) {
            flags = other.flags;
            length = other.length;
            text = other.text;
        } else {
            unsigned int count = isLong ? length : (*(unsigned char*)this & 0x7F);
            unsigned int otherCount;
            char* first;
            if (!(other.flags >> 31)) {
                otherCount = *(unsigned char*)&other & 0x7F;
                first = (char*)&other + 1;
            } else {
                first = other.text;
                otherCount = other.length;
            }
            fn_8030F460_Allocator allocator;
            fn_8030F460_Allocator copy = allocator;
            String_ReplaceRange(this, 0, count, first, first + otherCount, (char*)&copy);
        }
    }
};

struct fn_8030F460_Virtual {
    virtual ~fn_8030F460_Virtual();
};

struct fn_8030F460_Text {
    virtual ~fn_8030F460_Text();
    virtual void set(const char*);
    virtual void slot10();
    virtual void slot14();
    virtual int ready();
};

// Publish the BP script controller and register its destructor once.
static inline void EnsureBpController(char* base) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (base + IFF_OFFSET(DolfiniModuleStorage, bpDestructorRecord)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Publish the shared file-list owner and register its destructor once.
static inline void EnsureSharedFileList(char* base) {
    if (!Sing_805DC680()) {
        if (!s_sharedFileListInitialized) {
            SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
            __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                     (base + IFF_OFFSET(DolfiniModuleStorage, sharedFileDestructorRecord)));
            s_sharedFileListInitialized = 1;
        }
        Sing_805DC680() = s_sharedFileListStorage;
    }
}

// Publish the shared fade overlay and register its destructor once.
static inline void EnsureFadeOverlay(char* base) {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                     (base + IFF_OFFSET(DolfiniModuleStorage, fadeDestructorRecord)));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}

// Publish the shared input manager and register its destructor once.
static inline void EnsureInputManager(char* base) {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     base + IFF_OFFSET(DolfiniModuleStorage, inputDestructorRecord));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}

// Publish save data, constructing it once; the base argument is unused.
static inline void EnsureSaveData(char* base) {
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
}

// Publish the shared active-script handle and its destructor registration.
static inline void EnsureBpHandle(char* base) {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            *(int*)(s_bpCtrlHandle + 4) = -1;
            *(int*)s_bpCtrlHandle = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     (base + IFF_OFFSET(DolfiniModuleStorage, bpHandleDestructorRecord)));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
}

// Publish the loading indicator and register its destructor once.
static inline void EnsureLoadingIndicator(char* base) {
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)s_nowReloading)->Init();
            __register_global_object(s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     (base + IFF_OFFSET(DolfiniModuleStorage, loadingDestructorRecord)));
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = s_nowReloading;
    }
}

// Choose the scene finish animation/group for the current motion-speed tier.
static inline void SelectDolfiniFinishAnimation(PlayerRecordOwner8030E168* self) {
    if (6 == self->speedLevel) {
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(0xA);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(8);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
    } else if (5 == self->speedLevel) {
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(9);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(7);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
    } else if (self->speedLevel == 4) {
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(8);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(6);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
    } else {
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(7);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(5);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
    }
}

// Hide spin controls, show the capsule/dolphin shadows, and enter the finish wait.
static inline void EnterDolfiniFinishSpin(PlayerRecordOwner8030E168* self, char* base) {
    EnsureBpController(base);
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                               roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_02_OUT), 1);
    EnsureBpController(base);
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId6),
                               roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_OUT), 1);
    EnsureBpController(base);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5), 0);
    EnsureBpController(base);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId1), 0);
    ((CRoundDol3D*)(self->roundDol3D))->SetCapsuleShadowEnabled(1);
    ((CRoundDol3D*)(self->roundDol3D))->SetDolphinShadowEnabled(1);
    self->state = kDolfiniFinishSpin;
}

// Run the Dolfini prize game for one frame: load resources, run the title, accept a
// spin, award the prize, then replay or save and exit. Returns 1 when the session can
// close.
//
// The constructor and LoadResources must have run first. The save menu is stepped
// with a fixed 0.033 s step. Dialogs and save menus made here are deleted when they
// finish; the activation vector owns its array.
int PlayerRecordOwner8030E168::UpdateStateMachine() {
    char* name;
    char* base = s_roundDolInputDestructorRecord;
    // Copied to a local so the pointer is allocated the way the original does it.
    PlayerRecordOwner8030E168* self = (PlayerRecordOwner8030E168*)(void*)this;
    fn_8030F460_Coll arr;

    // Phase 1: only loading and prize/result states tick the layout.
    switch (self->state) {
    case kDolfiniWaitResources:
    case kDolfiniPrizeEnter:
    case kDolfiniChooseReplayOrExit:
    case kDolfiniPrizeLeaveForReplay:
    case kDolfiniPrizeLeaveForExit:
    case kDolfiniSaveAfterPrize:
        Layout_Update(self->layout);
        break;
    }

    // Phase 2: run one state. Waiting breaks reach the shared 3D update below.
    switch (self->state) {
        fn_8030EE64_String s;
        int ticketInventoryIndex;
        int selectedRewardId;
    // State 0: mute temporarily and warm scene animations 1..8, then await resources.
    case kDolfiniWarmAnimations:
        s_roundDolSavedBufferVolumeStorage = SoundValuePair_GetSelectedValue(g_soundManager, 0);
        SoundManager_SetBufferVolume(g_soundManager, kFloat_0_RoundDol, kFloat_0_RoundDol);
        // Warm up each animation.
        for (;;) {
            ((CRoundDol3D*)(self->roundDol3D))->UpdateAnimationAndCamera();
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetState())
                continue;
            self->preloadAnimationIndex = (1 + self->preloadAnimationIndex);
            if (self->preloadAnimationIndex <= 8) {
                ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(self->preloadAnimationIndex);
            } else {
                break;
            }
        }
        self->state = kDolfiniWaitResources;
        break;

    // Wait for every resource, then wrap the borrowed tables and create message drawing.
    case kDolfiniWaitResources:
        EnsureBpController(base);
        if (!((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded())
            break;
        if (!Layout_PollLoadComplete(self->layout))
            break;
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings))
            break;
        EnsureSharedFileList(base);
        {
            void* resource = Sing_805DC680();
            {
                fn_8030F460_String s;
                NString_AssignCString(&s, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
                strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))),
                       !(s.flags >> 31) ? (char*)&s + 1 : s.text);
                name = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
            }
            if (!FileList_GetLoadedBufferByName(resource, name))
                break;
        }
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(
                Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile)))
            break;
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(
                Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile)))
            break;
        self->resourcesLoaded = 1;
        EnsureFadeOverlay(base);
        FadeState_StartFadeOut(Sing_805DC608(), kDolfiniFadeFrames);
        EnsureLoadingIndicator(base);
        {
            void* p = Sing_805DC610();
            if (p && LoadingIndicator_IsAbsentOrState3(p))
                CB2dPlayerHandle::RequestPendingScript(p);
        }
        SoundManager_SetBufferVolume(g_soundManager, s_roundDolSavedBufferVolumeStorage, kFloat_0_RoundDol);
        self->state = kDolfiniTitleEnter;
        EnsureBpController(base);
        UIGroup_SetOpacity(
            UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0), kDolfiniTitleGroup), 0);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_IN), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId7),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, lensflare), 1);
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(1);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(0);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
        EnsureBpController(base);
        ((CFontNumDraw*)(self->numberDraw))
            ->RegisterEntries(roundDolPrizeNumericEntries, 2,
                              (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4),
                              roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        UITextTable_SetEntryResourceName(self->numberDraw, kDolfiniTicketNumberEntry,
                                         roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        UITextTable_SetEntryResourceName(self->numberDraw, kDolfiniPrizeNumberEntry,
                                         roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings);
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v)
                    p[0] = v;
                p[1] = __vt__9CGuruguru;
            }
            self->rewardTable = (int)p;
        }
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(
                Sing_805DC680(), (roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile)));
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v)
                    p[0] = v;
                p[1] = __vt__14CGallery_image;
            }
            self->galleryImageTable = (int)p;
        }
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(
                Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile));
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v)
                    p[0] = v;
                p[1] = __vt__14CPrize_gallery;
            }
            self->prizeGalleryTable = (int)p;
        }
        EnsureSharedFileList(base);
        {
            void* resource = Sing_805DC680();
            {
                fn_8030F460_String s;
                NString_AssignCString(&s, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
                strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))),
                       !(s.flags >> 31) ? (char*)&s + 1 : s.text);
                name = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
            }
            WFixedFont_CreateMsgEcbResource(self->textFactory, &self->messageResource,
                                            FileList_GetLoadedBufferByName(resource, name), 1);
        }
        WFixedFont_CreateAndRegisterMsgDraw(self->textFactory, &self->messageDraw, 1);
        ((CRoundDol3D*)(self->roundDol3D))->SetupModelShadows();
        Layout_SetElementEnabled(self->layout, kDolfiniPrizeImageElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniGalleryImageElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniPrizeTextElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniNoTicketsElement, 0);
        ((CProjectG*)g_project)->SetExtensionProperty(0x74C6EDFF);
        break;

    // State 2: await both title/intro scripts, then start the idle title and music.
    case kDolfiniTitleEnter:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        Layout_SetElementEnabled(self->layout, kDolfiniReplayElement, 1);
        Layout_SetElementEnabled(self->layout, kDolfiniExitElement, 1);
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0)))
            break;
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId7)))
            break;
        self->state = kDolfiniTitleIdle;
        EnsureBpController(base);
        UIGroup_SetOpacity(
            UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0), kDolfiniTitleGroup), 0xFF);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_MOV), 1);
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(2);
        SoundManager_RequestBgmChange(
            g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, spinning_dolph_wav), 0, 1);
        break;

    // State 3: confirm starts a paid attempt when a ticket is available.
    case kDolfiniTitleIdle:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureInputManager(base);
        if (!Input_IsActionPressed(GetInputManagerSlot(), kDolfiniConfirmAction, -1))
            break;
        if ((int)((CRoundDol3D*)(self->roundDol3D))->GetState())
            break;
        Sound_PlayByName(g_soundManager,
                         roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, party_character_decide), 0, 0, 0, 0,
                         kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
        ticketInventoryIndex =
            ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
        if (self->itemCounts[ticketInventoryIndex] <= 0) {
            {

                self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                             kDolfiniTicketItemId)] = 0;
            }
            {
                void* p = new fn_8030F460_Dialog(3, 0xF);
                self->dialog = p;
                ((CPangyaCommonDialog*)p)->SetText(roundDolDialogStrings);
            }
            EnsureInputManager(base);
            InputManager_ResumeCursorDrawing(GetInputManagerSlot());
            self->state = kDolfiniNoTicketsDialog;
            break;
        }
        self->state = kDolfiniTitleLeave;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_OUT), 1);
        EnsureBpController(base);
        UIGroup_SetOpacity(
            UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0), kDolfiniTitleGroup), 0);
        break;

    // State 4: await title exit, then reset the scene and bring in the arrow.
    case kDolfiniTitleLeave:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId0)))
            break;
        ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(3);
        ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(0);
        ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(0);
        self->state = kDolfiniArrowEnter;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId1),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_arrow_IN), 1);
        break;

    // State 5: await arrow entry, then bring in controls and instructions.
    case kDolfiniArrowEnter:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId1)))
            break;
        self->state = kDolfiniControlsEnter;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_IN), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_IN), 1);
        break;

    // State 6: await both entry scripts, then loop controls; decelerate each tick.
    case kDolfiniControlsEnter:
        EnsureBpController(base);
        if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2))) {
            EnsureBpController(base);
            if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3))) {
                self->state = kDolfiniWaitSpinInput;
                EnsureBpController(base);
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2),
                                           roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_MOV),
                                           1);
                EnsureBpController(base);
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                           roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_01_MOV),
                                           1);
            }
        }
        ((CRoundDol3D*)(self->roundDol3D))->DecelerateRotation();
        break;

    // State 7: cancel opens a confirmation; armed spin input starts the controls leaving.
    case kDolfiniWaitSpinInput:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureInputManager(base);
        if (Input_IsActionButtonDown(GetInputManagerSlot(), kDolfiniCancelAction, -1)) {
            {
                void* p = new fn_8030F460_Dialog(2, 0xA);
                self->dialog = p;
                ((CPangyaCommonDialog*)p)->SetText(roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, confirmExit));
            }
            self->state = kDolfiniConfirmExit;
            EnsureInputManager(base);
            InputManager_ResumeCursorDrawing(GetInputManagerSlot());
            break;
        }
        EnsureInputManager(base);
        if (Input_IsActionPressed(GetInputManagerSlot(), kDolfiniSpinAction, -1))
            self->spinInputArmed = 1;
        EnsureInputManager(base);
        if (Input_IsActionButtonDown(GetInputManagerSlot(), kDolfiniSpinAction, -1) && self->spinInputArmed) {
            Layout_SetElementEnabled(self->layout, kDolfiniReplayElement, 1);
            Layout_SetElementEnabled(self->layout, kDolfiniExitElement, 1);
            self->state = kDolfiniControlsLeave;
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_OUT), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_OUT), 1);
        }
        ((CRoundDol3D*)(self->roundDol3D))->DecelerateRotation();
        break;

    // State 8: await both exit scripts before enabling spin-start state.
    case kDolfiniControlsLeave:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2)))
            break;
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3)))
            break;
        self->state = kDolfiniWaitSpinStart;
        break;

    // State 9: recheck instruction completion, then accept motion-driven spinning.
    case kDolfiniWaitSpinStart:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3)))
            break;
        self->state = kDolfiniSpinning;
        break;

    // State 10: update speed effects, test peak drop, then test stopped motion/retry.
    case kDolfiniSpinning:
        // Speed-specific animation transitions.
        switch ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel()) {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == self->speedLevel)
                break;
            if (self->peakSpeedLevel < (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel())
                self->peakSpeedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            self->speedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            EnsureBpController(base);
            BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_02_MOV), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_A), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId6),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_MOV), 1);
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(2);
            break;
        case 4:
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == self->speedLevel)
                break;
            if (self->peakSpeedLevel < (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel())
                self->peakSpeedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            self->speedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_B), 1);
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(3);
            break;
        case 5:
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == self->speedLevel)
                break;
            if (self->peakSpeedLevel < (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel())
                self->peakSpeedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            self->speedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_C), 1);
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(4);
            break;
        case 6:
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == self->speedLevel)
                break;
            if (self->peakSpeedLevel < (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel())
                self->peakSpeedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            self->speedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_D), 1);
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(5);
            break;
        case 7:
            if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == self->speedLevel)
                break;
            if (self->peakSpeedLevel < (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel())
                self->peakSpeedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            self->speedLevel = (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel();
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId5),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_E), 1);
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(6);
            break;
        default:
            ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(6);
            break;
        }
        // Peak 7 ends on the first drop; lower peaks end after a two-level drop.
        if (self->peakSpeedLevel == 7) {
            if (self->peakSpeedLevel != (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel()) {
                ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(7);
                ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(5);
                ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(1);
                EnterDolfiniFinishSpin(self, base);
            }
        } else if (self->peakSpeedLevel == (int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() + 2) {
            SelectDolfiniFinishAnimation(self);
            EnterDolfiniFinishSpin(self, base);
        }
        // If motion stops at peak <=2, retry the controls without consuming a ticket.
        if (((CRoundDol3D*)(self->roundDol3D))->UpdateRotationFromPadMotion())
            break;
        if (2 >= self->peakSpeedLevel) {
            self->peakSpeedLevel = 0;
            self->state = kDolfiniControlsEnter;
            self->speedLevel = 0;
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_IN), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                       roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_IN), 1);
            break;
        }
        SelectDolfiniFinishAnimation(self);
        EnterDolfiniFinishSpin(self, base);
        break;

    // State 11: wait for the scene and messages, debit one ticket, award the prize and
    // build the result UI.
    case kDolfiniFinishSpin:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureBpController(base);
        if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3))) {
            EnsureBpController(base);
            if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId6)) &&
                !(int)((CRoundDol3D*)(self->roundDol3D))->GetState()) {
                {
                    fn_8030F460_String prizeResourceName;
                    fn_8030F460_String prizeResourceStem;

                    EnsureBpHandle(base);
                    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(0x13);
                    ticketInventoryIndex =
                        ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
                    --self->itemCounts[ticketInventoryIndex];
                    selectedRewardId = Dolfini_SelectAndApplyWeightedReward((struct DolfiniRewardState*)self);
                    // Build image/text resources for inventory kind 0 or gallery kind !=0.
                    if (self->rewardKind == 0) {
                        char* p = ((IffItemCommon*)ItemInventory_FindDataWithHairStyleFallback(
                                       AsCSharedDoc(g_sharedDoc)->itemInventory, selectedRewardId))
                                      ->icon;
                        unsigned int count = prizeResourceStem.size();
                        fn_8030F460_Allocator allocator;
                        fn_8030F460_Allocator copy = allocator;
                        String_ReplaceRange(&prizeResourceStem, 0, count, p, p + strlen(p), (char*)&copy);
                        {
                            fn_8030F460_String t3(0);
                            fn_8030F460_String t4(0);
                            String_ConcatCStringAndString(
                                &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, zeroBytes_227),
                                &prizeResourceStem);
                            String_ConstructConcatCStr(
                                &t4, &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_gsr));
                            prizeResourceName.assign(t4);
                        }
                        BpTextureBinding_ReleaseSources(self->texture0);
                        ((fn_8030F460_Text*)self->texture0)->set(prizeResourceName.data());
                        {
                            float textPosition[2];
                            fn_8030F460_String wide;

                            *(int*)&textPosition[0] = kRoundDolItemTextInitialXBits;
                            *(int*)&textPosition[1] = kRoundDolItemTextInitialYBits;
                            Layout_GetElementPosition(self->layout, kDolfiniPrizeTextElement, textPosition);
                            textPosition[0] = kFloat_0_RoundDol;
                            Text_SetPosition((void*)self->messageDraw, textPosition);
                            ((CMsgDraw*)(self->messageDraw))->ResetTextState();
                            ((CMsgDraw*)(self->messageDraw))->ClearFormattingAndRevealAll();
                            ((CMsgDraw*)(self->messageDraw))->SetColor(kDolfiniPrizeTextColor);
                            {
                                char* q = roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, prizeMessagePrefix);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;
                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, q, q + wcslen(q) * 2, (char*)&copy);
                            }
                            {
                                fn_8030F460_Allocator allocator;
                                char* p = (char*)MessageTable_FindTextByName(
                                    (void*)self->messageResource,
                                    ItemInventory_FindDataById(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                               selectedRewardId));
                                unsigned int count = wide.size();

                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, p, p + wcslen(p) * 2, (char*)&copy);
                            }
                            Text_SetContent((void*)self->messageDraw,
                                            !(wide.flags >> 31) ? (char*)&wide + 2 : wide.text);
                            fn_8030F460_Allocator allocator;
                            fn_8030F460_Allocator copy = allocator;
                            WString_ReplaceRange(&wide, 0, -1, 0, 0, (char*)&copy);
                        }
                    } else {
                        fn_8030F460_Allocator allocator;
                        unsigned int galleryImageId =
                            PrizeGallery_GetImageId((void*)self->prizeGalleryTable, selectedRewardId);
                        void* imageTable = (void*)self->galleryImageTable;
                        unsigned int imageRecordOffset =
                            SharedFileTable_GetMappedValueAt(imageTable, (short)galleryImageId);
                        void* imageRecord = Table_ResolveDataOffset(*(void**)imageTable, imageRecordOffset);
                        char* imageResourceStem = imageRecord ? (char*)OffsetStringNode_GetText(imageRecord) : 0;
                        unsigned int count = prizeResourceStem.size();

                        fn_8030F460_Allocator copy = allocator;
                        String_ReplaceRange(&prizeResourceStem, 0, count, imageResourceStem,
                                            imageResourceStem + strlen(imageResourceStem), (char*)&copy);
                        {
                            fn_8030F460_String t4(0);
                            fn_8030F460_String t3(0);
                            String_ConcatCStringAndString(
                                &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, zeroBytes_227),
                                &prizeResourceStem);
                            String_ConstructConcatCStr(
                                &t4, &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, tmb_gsr));
                            prizeResourceName.assign(t4);
                        }
                        fn_8030F460_Text* galleryTexture = (fn_8030F460_Text*)self->texture1;
                        BpTextureBinding_ReleaseSources(self->texture0);
                        ((fn_8030F460_Text*)self->texture0)
                            ->set(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_res_item_00_gsr));
                        BpTextureBinding_ReleaseSources(galleryTexture);
                        ((fn_8030F460_Text*)self->texture1)->set(prizeResourceName.data());
                        {
                            fn_8030F460_String wide;
                            float textPosition[2];
                            *(int*)&textPosition[0] = kRoundDolGalleryTextInitialXBits;
                            *(int*)&textPosition[1] = kRoundDolGalleryTextInitialYBits;
                            Layout_GetElementPosition(self->layout, kDolfiniPrizeTextElement, textPosition);
                            textPosition[0] = kFloat_0_RoundDol;
                            Text_SetPosition((void*)self->messageDraw, textPosition);
                            ((CMsgDraw*)(self->messageDraw))->ResetTextState();
                            ((CMsgDraw*)(self->messageDraw))->ClearFormattingAndRevealAll();
                            ((CMsgDraw*)(self->messageDraw))->SetColor(kDolfiniPrizeTextColor);
                            {
                                char* q = roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, prizeMessagePrefix);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;
                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, q, q + wcslen(q) * 2, (char*)&copy);
                            }
                            {

                                void* prizeTable = (void*)self->prizeGalleryTable;
                                int messageRecordOffset =
                                    PrizeGallery_FindMessageRecordOffset(prizeTable, selectedRewardId);
                                void* messageRecord = Table_ResolveDataOffset(*(void**)prizeTable, messageRecordOffset);
                                void* messageKey = messageRecord ? OffsetStringNode_GetText(messageRecord) : 0;
                                char* messagePayload =
                                    (char*)TextResource_FindNamedEntry((void*)self->messageResource, messageKey);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;

                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, messagePayload,
                                                     messagePayload + wcslen(messagePayload) * 2, (char*)&copy);
                            }
                            Text_SetContent((void*)self->messageDraw,
                                            !(wide.flags >> 31) ? (char*)&wide + 2 : wide.text);
                            fn_8030F460_Allocator allocator;
                            fn_8030F460_Allocator copy = allocator;
                            WString_ReplaceRange(&wide, 0, -1, 0, 0, (char*)&copy);
                        }
                    }
                    ((CRoundDol3D*)(self->roundDol3D))->SetCapsuleShadowEnabled(0);
                    self->state = kDolfiniPrizeEnter;
                    EnsureBpController(base);
                    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4),
                                               roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_IN),
                                               1);
                    EnsureInputManager(base);
                    InputManager_ResumeCursorDrawing(GetInputManagerSlot());
                }
                break;
            }
        }
        ((CRoundDol3D*)(self->roundDol3D))->DecelerateRotation();
        break;

    // State 12: wait for result script/image, then enable choices (disable replay at zero tickets).
    case kDolfiniPrizeEnter:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4)))
            break;
        if (!((fn_8030F460_Text*)self->texture0)->ready())
            break;
        self->state = kDolfiniChooseReplayOrExit;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_MOV), 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniReplayElement, 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniExitElement, 1);
        ticketInventoryIndex =
            ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
        if (self->itemCounts[ticketInventoryIndex] > 0)
            break;
        {

            self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                         kDolfiniTicketItemId)] = 0;
        }
        Layout_SetElementPlayerFlag(self->layout, kDolfiniReplayElement, 0, 5);
        Layout_SetElementEnabled(self->layout, kDolfiniNoTicketsElement, 1);
        break;

    // State 13: exit commits the working record to save data; replay resets the scene,
    // the speed counters and the result UI.
    case kDolfiniChooseReplayOrExit: {
        int i;
        Layout_CollectActivated(self->layout, &arr, 0);
        for (i = 0; arr.size > i; i++) {
            if (((int*)arr.data)[i] == kDolfiniExitElement) {
                Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniExitElement, 0);
                Layout_SetElementPlayerFlag(self->layout, kDolfiniReplayElement, 0, 5);
                self->state = kDolfiniSaveAfterPrize;
                EnsureFadeOverlay(base);
                FadeState_RequestFramebufferCapture(Sing_805DC608());
                EnsureSaveData(base);
                SaveData_SetCurrentRecord(GetSaveDataSlot(), self->savedPlayerRecord);
                self->saveMenu = CFlashMemoryMenuSave::Create();
                SoundManager_RequestBgmStop(g_soundManager, 1);
            }
            if (((int*)arr.data)[i] == kDolfiniReplayElement) {
                ((CRoundDol3D*)(self->roundDol3D))->ResetRotation();
                Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniReplayElement, 0);
                Layout_SetElementPlayerFlag(self->layout, kDolfiniExitElement, 0, 5);
                self->state = kDolfiniPrizeLeaveForReplay;
                self->speedLevel = 0;
                self->peakSpeedLevel = 0;
                ((CRoundDol3D*)(self->roundDol3D))->ResetRotation();
                ((CMsgDraw*)(self->messageDraw))->ResetTextState();
                ((CRoundDol3D*)(self->roundDol3D))->StartGroupPlayback(4);
                ((CRoundDol3D*)(self->roundDol3D))->SetCameraTargetOverrideEnabled(0);
                ((CRoundDol3D*)(self->roundDol3D))->SetAnimationByIndex(0);
                EnsureBpController(base);
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4),
                                           roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_OUT),
                                           1);
            }
        }
        break;
    }

    // State 14: wait for result exit; restore controls and suspend cursor drawing.
    case kDolfiniPrizeLeaveForReplay:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4)))
            break;
        self->state = kDolfiniWaitSpinInput;
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId1), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_MOV), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_01_MOV), 1);
        EnsureInputManager(base);
        InputManager_SuspendCursorDrawing(GetInputManagerSlot());
        EnsureBpHandle(base);
        ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
        ((CRoundDol3D*)(self->roundDol3D))->SetDolphinShadowEnabled(1);
        break;

    // State 15: await result exit and report completion without a save-menu tick.
    case kDolfiniPrizeLeaveForExit:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId4)))
            break;
        return 1;

    // State 16: dialog result 1 saves and exits; result 2 restores the controls and
    // clears the spin latch.
    case kDolfiniConfirmExit:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        if (!(*(int (**)(void*, float))(*(char**)self->dialog + 0x10))(self->dialog, kFloat_0_RoundDol))
            break;
        if (((CPangyaCommonDialog*)self->dialog)->result == 1) {
            if (self->dialog) {
                delete (fn_8030F460_Virtual*)self->dialog;
                self->dialog = 0;
            }
            EnsureSaveData(base);
            SaveData_SetCurrentRecord(GetSaveDataSlot(), self->savedPlayerRecord);
            self->saveMenu = CFlashMemoryMenuSave::Create();
            SoundManager_RequestBgmStop(g_soundManager, 1);
            self->state = kDolfiniSaveAfterCancel;
            EnsureFadeOverlay(base);
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            break;
        }
        if (((CPangyaCommonDialog*)self->dialog)->result != 2)
            break;
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId2), 1);
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId3), 1);
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (void*)self->bpEntryId1), 1);
        self->state = kDolfiniControlsEnter;
        if (self->dialog) {
            delete (fn_8030F460_Virtual*)self->dialog;
            self->dialog = 0;
        }
        EnsureInputManager(base);
        InputManager_SuspendCursorDrawing(GetInputManagerSlot());
        self->spinInputArmed = 0;
        break;

    // State 17: poll the save UI; delete it on completion, otherwise keep the fade active.
    case kDolfiniSaveAfterPrize:
        if (MPBase1_Process(self->saveMenu, kFloat_0_033_RoundDolMain)) {
            if (self->saveMenu) {
                delete (fn_8030F460_Virtual*)self->saveMenu;
                self->saveMenu = 0;
            }
            return 1;
        }
        EnsureFadeOverlay(base);
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        EnsureFadeOverlay(base);
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        break;

    // State 18: the same save contract after a cancelled spin.
    case kDolfiniSaveAfterCancel:
        if (MPBase1_Process(self->saveMenu, kFloat_0_033_RoundDolMain)) {
            if (self->saveMenu) {
                delete (fn_8030F460_Virtual*)self->saveMenu;
                self->saveMenu = 0;
            }
            return 1;
        }
        EnsureFadeOverlay(base);
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        EnsureFadeOverlay(base);
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        break;

    // State 20: close the no-ticket dialog, start fade-in and report completion.
    case kDolfiniNoTicketsDialog:
        if ((*(int (**)(void*, float))(*(char**)self->dialog + 0x10))(self->dialog, kFloat_0_RoundDol)) {
            if (self->dialog) {
                delete (fn_8030F460_Virtual*)self->dialog;
                self->dialog = 0;
            }
            EnsureFadeOverlay(base);
            FadeState_StartFadeIn(Sing_805DC608(), 0);
            return 1;
        }
        EnsureFadeOverlay(base);
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        break;

    // State 19: stay idle; still reach the shared scene update.
    case kDolfiniInactive:
        break;
    }

    // Shared frame update.
    ((CRoundDol3D*)(self->roundDol3D))->UpdateAnimationAndCamera();
    return 0;
}

// Enqueues asynchronous resources, retaining BP entry IDs in self. The shared
// file-list owns queued files; textFactory is borrowed from the chat-message owner.
// Queue the prize-game UI/table resources and suspend pointer drawing while the scene loads.
void DolfiniSession_LoadResources(void* self) {
    char* base = s_roundDolInputDestructorRecord;
    EnsureBpController(base);
    // Phase 1: enqueue the eight BP layers in title/arrow/controls/message/result order.
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_b2d), self, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_02_ARO_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId1, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_01_COS_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId2, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_00_MSG_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId3, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId4, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId5, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId6, 0);
    EnsureBpController(base);
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dolph_tojyo_b2d),
                      &AsPlayerRecordOwner8030E168(self)->bpEntryId7, 0);
    // Phase 2: load pointer layout and 3D scene, then queue message and reward tables.
    Layout_InitStandard(AsPlayerRecordOwner8030E168(self)->layout, 0, roundDolLayoutElements, 8,
                        roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_spd));
    ((CRoundDol3D*)(AsPlayerRecordOwner8030E168(self)->roundDol3D))->LoadSceneResources();
    EnsureSharedFileList(base);
    void* cached680 = Sing_805DC680();
    fn_8030EE64_String local = {};
    NString_AssignCString(&local, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
    strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))),
           !(local.flags >> 31) ? (char*)&local + 1 : local.text);
    char* dst = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
    if (local.flags >> 31)
        appFreeUni__FPv(local.text);
    SharedFileList_CreateAndAppend(cached680, dst, 0);
    AsPlayerRecordOwner8030E168(self)->textFactory = ((CChatMsg*)g_chatMsg)->defaultFixedFont;
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(), roundDolPrizeStrings, 0);
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile), 0);
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(),
                                   roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile), 0);
    // Phase 3: stop previous music and hide pointer drawing while resources load.
    SoundManager_RequestBgmStop(g_soundManager, 1);
    EnsureInputManager(base);
    InputManager_SuspendCursorDrawing(GetInputManagerSlot());
}

// Ensure the BP controller used by state-specific UI drawing exists.
static inline void EnsureDrawBpController() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_roundDolDrawBpDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

#pragma push
#pragma always_inline on
// Render the 3D shadow pass before the visible models/effects.
static inline void DrawDolfiniScene(void* self) {
    ((CRoundDol3D*)(AsPlayerRecordOwner8030E168(self)->roundDol3D))->RenderShadowPass();
    ((CRoundDol3D*)(AsPlayerRecordOwner8030E168(self)->roundDol3D))->DrawModelsAndEffects();
}
// Resolve a borrowed BP entry after ensuring its registry, then invoke its callback.
static inline void InvokeDolfiniEntryCallback(int* entryId) {
    EnsureDrawBpController();
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), (void*)*entryId), 0);
}
// Resolve a borrowed BP entry after ensuring its registry, then draw it.
static inline void DrawDolfiniEntry(int* entryId) {
    EnsureDrawBpController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), (void*)*entryId));
}
#pragma pop

// Draw the supplied quad template over the gallery-image layout element.
static void DrawGalleryQuad(void* self, const char* vertices) {
    float position[4];
    float size[4];
    float verticesOut[4][6];
    Layout_GetElementPosition(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniGalleryImageElement, position);
    Layout_GetElementSize(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniGalleryImageElement, size);
    for (int i = 0; i < 12; ++i) {
        ((unsigned int*)verticesOut)[i] = ((unsigned int*)vertices)[i];
    }
    verticesOut[0][0] = position[0];
    verticesOut[0][1] = position[1];
    verticesOut[1][0] = position[0] + size[0];
    verticesOut[1][1] = position[1];
    verticesOut[2][0] = position[0];
    verticesOut[2][1] = position[1] + size[1];
    verticesOut[3][0] = position[0] + size[0];
    verticesOut[3][1] = position[1] + size[1];
    Engine_SetFlexibleVertexFormat(kDolfiniTextureVertexFormat);
    void* buffer = GX_GetPrimitiveCount(kDolfiniQuadVertices, kDolfiniTriangleStrip);
    DrawPrimitiveUP(kDolfiniTriangleStrip, buffer, &verticesOut[0][0], kDolfiniQuadVertexBytes);
}

// Layer order is part of the presentation: shadow/3D, all relevant callbacks,
// then BP drawings. Prize states use shadow + result/layout + gallery/item quads.
// Confirm/no-ticket dialogs draw over their underlying scene; the save menu draws
// last in any state. This routine borrows every resource and advances no session state.
// Draw the 3D scene and UI appropriate to the session state, including prize images and dialogs.
void CRoundDolMain::DrawStateUI() {
    UIRender_SetDefaultState(this);
    // Skip state-specific drawing until ready; cleanup remains shared (entry 0x80312CF0).
    if (AsPlayerRecordOwner8030E168(this)->resourcesLoaded != 0) {
        unsigned int state = AsPlayerRecordOwner8030E168(this)->state;
        switch (state) {
        case kDolfiniTitleEnter:
        case kDolfiniTitleIdle:
        case kDolfiniTitleLeave:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId0);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId7);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId0);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId7);
            break;
        case kDolfiniArrowEnter:
        case kDolfiniControlsEnter:
        case kDolfiniWaitSpinInput:
        case kDolfiniControlsLeave:
        case kDolfiniWaitSpinStart:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            break;
        case kDolfiniSpinning:
        case kDolfiniFinishSpin:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId6);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId5);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId6);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId5);
            break;
        // Images use layout XY/size; templates preserve Z, packed color, and UV.
        case kDolfiniPrizeEnter:
        case kDolfiniChooseReplayOrExit:
        case kDolfiniPrizeLeaveForReplay:
        case kDolfiniPrizeLeaveForExit:
        case kDolfiniSaveAfterPrize:
            ((CRoundDol3D*)(AsPlayerRecordOwner8030E168(this)->roundDol3D))->RenderShadowPass();
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId4);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId4);
            Layout_DrawWithOpacity(AsPlayerRecordOwner8030E168(this)->layout, 1.0f);
            if (AsPlayerRecordOwner8030E168(this)->rewardKind) {
                if (((int (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(this)->texture1))[6])(
                        AsPlayerRecordOwner8030E168(this)->texture1)) {
                    ((void (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(this)->texture1))[5])(
                        AsPlayerRecordOwner8030E168(this)->texture1);
                    {
                        float position[4];
                        float size[4];
                        float v[4][6];
                        Layout_GetElementPosition(AsPlayerRecordOwner8030E168(this)->layout,
                                                  kDolfiniGalleryImageElement, position);
                        Layout_GetElementSize(AsPlayerRecordOwner8030E168(this)->layout, kDolfiniGalleryImageElement,
                                              size);
                        memcpy((char*)v, roundDolGalleryQuadTemplate, 0x60);
                        v[0][0] = position[0];
                        v[0][1] = position[1];
                        v[1][0] = position[0] + size[0];
                        v[1][1] = position[1];
                        v[2][0] = position[0];
                        v[2][1] = position[1] + size[1];
                        v[3][0] = position[0] + size[0];
                        v[3][1] = position[1] + size[1];
                        Engine_SetFlexibleVertexFormat(kDolfiniTextureVertexFormat);
                        void* buffer = GX_GetPrimitiveCount(kDolfiniQuadVertices, kDolfiniTriangleStrip);
                        DrawPrimitiveUP(kDolfiniTriangleStrip, buffer, &v[0][0], kDolfiniQuadVertexBytes);
                    }
                }
            }
            {
                if (((int (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(this)->texture0))[6])(
                        AsPlayerRecordOwner8030E168(this)->texture0)) {
                    ((void (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(this)->texture0))[5])(
                        AsPlayerRecordOwner8030E168(this)->texture0);
                    {
                        float position[4];
                        float size[4];
                        float v[4][6];
                        Layout_GetElementPosition(AsPlayerRecordOwner8030E168(this)->layout, kDolfiniPrizeImageElement,
                                                  position);
                        Layout_GetElementSize(AsPlayerRecordOwner8030E168(this)->layout, kDolfiniPrizeImageElement,
                                              size);
                        memcpy((char*)v, roundDolPrizeQuadTemplate, 0x60);
                        v[0][0] = position[0];
                        v[0][1] = position[1];
                        v[1][0] = position[0] + size[0];
                        v[1][1] = position[1];
                        v[2][0] = position[0];
                        v[2][1] = position[1] + size[1];
                        v[3][0] = position[0] + size[0];
                        v[3][1] = position[1] + size[1];
                        Engine_SetFlexibleVertexFormat(kDolfiniTextureVertexFormat);
                        void* buffer = GX_GetPrimitiveCount(kDolfiniQuadVertices, kDolfiniTriangleStrip);
                        DrawPrimitiveUP(kDolfiniTriangleStrip, buffer, &v[0][0], kDolfiniQuadVertexBytes);
                    }
                }
            }
            break;
        case kDolfiniConfirmExit:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            ((void (*)(void*))(*(void***)AsPlayerRecordOwner8030E168(this)->dialog)[5])(
                AsPlayerRecordOwner8030E168(this)->dialog);
            break;
        case kDolfiniSaveAfterCancel:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId2);
            /* original case 18 (jump table 0x804B8484 -> 0x803138B4) ends at 0x80313B10 with b to the 0x894 check: no slot-0x14 call on +0x880 */
            break;
        case kDolfiniNoTicketsDialog:
            DrawDolfiniScene(this);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId0);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(this)->bpEntryId7);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId0);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(this)->bpEntryId7);
            ((void (*)(void*))(*(void***)AsPlayerRecordOwner8030E168(this)->dialog)[5])(
                AsPlayerRecordOwner8030E168(this)->dialog);
            break;
        case kDolfiniWarmAnimations:
        case kDolfiniWaitResources:
        case kDolfiniInactive:

            break;
        }
    }
    if (AsPlayerRecordOwner8030E168(this)->saveMenu)
        MPBase1_DrawIfInitialized(AsPlayerRecordOwner8030E168(this)->saveMenu);
}

void __dl__FPv(void*);

// On-disk reward payload (16 bytes): +2 kind (0 inventory, nonzero gallery),
// +4 numeric reward ID stored as a pointer-sized word, +8 quantity, +A weight,
// +C rare-pool flag, +E enabled flag. Weights/quantities are signed shorts.
// enabled controls eligibility; rare chooses the pool, not a multiplier.
struct DolfiniRewardEntry {
    unsigned char padding00[2];
    short rewardKind;
    void* rewardId;
    short quantity;
    short weight;
    short rare;
    short enabled;
};

int time(int);
void srand(int);
int rand();
DolfiniRewardEntry* Table_GetEntry(void*, int);
void UITextTable_SetInteger(void*, int, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
int ItemInventory_FindAuxPartIndex(void*, unsigned int);

extern void* g_soundManager;
extern CSharedDoc* g_sharedDoc;
extern float kFloat_0_5_RoundDol;
extern float kFloat_200_RoundMenu;
extern float kFloat_1_RoundDolSound;
extern char roundDolPrizeStrings[];

struct DolfiniRewardList {
    unsigned char padding00[8];
    int count;
};

struct DolfiniRewardTable {
    DolfiniRewardList* list;
};

struct DolfiniRewardState {
    unsigned char padding00[0x38];
    void* roundDol3D;
    void* numberDraw;
    DolfiniRewardTable* rewardTable;
    unsigned char padding44[8];
    int rewardKind;
    unsigned char padding50[0x39];
    signed char equippedItemIndices[8];
    unsigned char padding91[0xE3];
    short itemCounts[13];
};

// Look up one entry in the session's reward table.
static DolfiniRewardEntry* GetRewardEntry(DolfiniRewardState* self, int index) {
    return Table_GetEntry(self->rewardTable->list, index);
}

// Return the loaded reward-table entry count, or zero while its list is absent.
static int GetRewardEntryCount(DolfiniRewardState* self) {
    DolfiniRewardList* list = self->rewardTable->list;
    return list ? list->count : 0;
}

// Draw the ordinary-pool weight from TEN random values; rare draws use ONE.
static inline int DrawOrdinaryDolfiniWeight(int totalWeight) {
    int sum = 0;
    for (int draw = 0; draw < kDolfiniRandomDraws; ++draw)
        sum += rand();
    return sum % totalWeight;
}

// Preconditions: the selected rare/ordinary pool has positive total weight,
// inventory IDs resolve, and the reward table is loaded. The routine reseeds rand
// from time each call. Rare chance is 1/100, or 1/50 at speed level 6.
// Returns the numeric reward ID; self->rewardKind tells callers which table owns
// that ID. Kind 0 updates inventory, while gallery kinds only configure the result.
// Select an enabled weighted prize, apply inventory rewards with count caps, update the UI, and return its ID.
int Dolfini_SelectAndApplyWeightedReward(DolfiniRewardState* self) {
    int rewardKind;
    int prizeQuantity;
    int ordinaryTableOrdinal;
    int rewardId;
    int totalWeight;
    totalWeight = 0;
    srand(time(0));
    int divisor = 1;
    if ((int)((CRoundDol3D*)(self->roundDol3D))->GetSpeedLevel() == 6)
        divisor = 2;
    int randomValue = rand();

    // Weight-edit contract: enabled==0 excludes a row; rare==0 selects the ordinary
    // pool and any nonzero rare value selects the rare pool. BOTH the sum and scan
    // must use the same membership rule. quantity affects inventory, never weight.
    // Use nonnegative weights for the usual weighted-draw interpretation; the
    // chosen pool must have a positive sum. Neither loop validates table edits.
    // Phase 1: select the rare/ordinary pool, sum enabled weights, then walk a draw.
    if (randomValue % (kDolfiniRarePercentBase / divisor) == 0) {
        int i = 0;
        while (i < (self->rewardTable->list ? self->rewardTable->list->count : 0)) {
            DolfiniRewardEntry* item = Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, i);
            short weight = item->weight;
            short enabled = item->enabled;
            short isRarePool = item->rare;
            if (0 != enabled && 0 != isRarePool)
                totalWeight += weight;
            i += 1;
        }
        int remainingWeight = rand() % totalWeight;
        int rareTableOrdinal = 0;
        for (;;) {
            if (rareTableOrdinal == GetRewardEntryCount(self))
                rareTableOrdinal = 0;
            DolfiniRewardEntry* item =
                Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, rareTableOrdinal);

            short weight = item->weight;
            short isRarePool = item->rare;
            if ((item->enabled) != 0 && 0 != isRarePool)
                remainingWeight -= weight;
            if (remainingWeight < 0) {
                DolfiniRewardEntry* chosen = Table_GetEntry(self->rewardTable->list, rareTableOrdinal);
                // Copy prize fields; the scan ordinal is never used as an inventory quantity.
                prizeQuantity = chosen->quantity;
                rewardKind = chosen->rewardKind;
                rewardId = *(int*)&chosen->rewardId;
                self->rewardKind = rewardKind;
                UITextTable_SetInteger(self->numberDraw, kDolfiniPrizeNumberEntry, prizeQuantity);
                break;
            }
            rareTableOrdinal += 1;
        }
        Sound_PlayByName(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, rarePrizeSound), 0,
                         0, 0, 0, kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
    } else {
        int i = 0;
        while (i < (self->rewardTable->list ? self->rewardTable->list->count : 0)) {
            DolfiniRewardEntry* item = Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, i);

            short weight = item->weight;
            short isRarePool = item->rare;
            if ((item->enabled) != 0 && !isRarePool)
                totalWeight += weight;
            i = (i + 1);
        }
        // Ordinary draws sum ten rand() results; rare draws use one. Preserve the
        // number/order of calls when tuning weights to retain RNG behavior.
        int ordinaryDraw = DrawOrdinaryDolfiniWeight(totalWeight);

        // The ordinary scan ordinal and the eventual prize quantity have separate roles.
        ordinaryTableOrdinal = 0;
        int remainingWeight = ordinaryDraw;
        for (;;) {
            if (ordinaryTableOrdinal == GetRewardEntryCount(self))
                ordinaryTableOrdinal = 0;
            DolfiniRewardEntry* item =
                Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, ordinaryTableOrdinal);

            short weight = item->weight;
            short isRarePool = item->rare;
            if (0 != (item->enabled) && isRarePool == 0)
                remainingWeight -= weight;
            if (0 > remainingWeight) {
                DolfiniRewardEntry* chosen = Table_GetEntry(self->rewardTable->list, ordinaryTableOrdinal);
                // Copy prize fields; the scan ordinal is never used as an inventory quantity.
                prizeQuantity = chosen->quantity;
                rewardKind = chosen->rewardKind;
                rewardId = (int)chosen->rewardId;
                self->rewardKind = rewardKind;
                UITextTable_SetInteger(self->numberDraw, kDolfiniPrizeNumberEntry, prizeQuantity);
                break;
            }
            ordinaryTableOrdinal += 1;
        }
        Sound_PlayByName(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, cookiePrizeSound), 0,
                         0, 0, 0, kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
    }
    // Phase 2: inventory rewards set auxiliary ownership, or add quantities with a
    // 999-item cap that includes all matching equipped slots. Gallery rewards skip this.
    if (rewardKind == kDolfiniInventoryReward) {
        if ((unsigned int)rewardId >> 26 == kDolfiniAuxPartKind) {

            ((DolfiniInventoryView*)self)
                ->auxiliaryParts[ItemInventory_FindAuxPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                                (unsigned int)rewardId)] = 1;
        } else {
            int equippedCount = 0;
            for (int i = 0; i < kDolfiniEquippedItemSlots; ++i) {
                if (self->equippedItemIndices[i] ==
                    ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId))
                    ++equippedCount;
            }
            int total = equippedCount + self->itemCounts[ItemInventory_FindItemIndex(
                                            AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)];
            int countAfterPrize = prizeQuantity + total;
            if (countAfterPrize >= kDolfiniItemLimit) {
                self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                             (unsigned int)rewardId)] =
                    (short)(kDolfiniItemLimit - equippedCount);
            } else {
                self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                             (unsigned int)rewardId)] += prizeQuantity;
            }
        }
    }
    // Phase 3: normalize the remaining ticket count to [0,999] and refresh the counter.
    if (self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] <=
        0) {
        self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] =
            0;
    }
    if (self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] >
        kDolfiniItemLimit) {
        self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] =
            kDolfiniItemLimit;
    }
    UITextTable_SetInteger(
        self->numberDraw, kDolfiniTicketNumberEntry,
        self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)]);
    return rewardId;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_8() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_7() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_6() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_5() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_4() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_3() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_2() {
    return 1;
}

// Default table callback reports success without additional work.
int CGuruguru::vfunc_1() {
    return 1;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CGuruguru::vfunc_0(int flag) {
    if (this && flag > 0)
        __dl__FPv(this);
    return this;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CGallery::image_vfunc_0(int flag) {
    if (this && flag > 0)
        __dl__FPv(this);
    return this;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CPrize::gallery_vfunc_0(int flag) {
    if (this && flag > 0)
        __dl__FPv(this);
    return this;
}

} // extern "C"
