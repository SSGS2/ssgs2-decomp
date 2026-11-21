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
    char rewardTableFile[9]; // 0x0
    char galleryImageTableFile[18]; // 0x9
    char prizeGalleryTableFile[18]; // 0x1B
    char iff_msg_eky[12]; // 0x2D
    char dlh_title_b2d[14]; // 0x39
    char dlh_ingame_02_ARO_b2d[22]; // 0x47
    char dlh_ingame_01_COS_b2d[22]; // 0x5D
    char dlh_ingame_00_MSG_b2d[22]; // 0x73
    char dlh_result_b2d[15]; // 0x89
    char dlh_eps_b2d[12]; // 0x98
    char dlh_hukidasi_b2d[17]; // 0xA4
    char dolph_tojyo_b2d[16]; // 0xB5
    char dlh_result_spd[15]; // 0xC5
    char dlh_title_waku_IN[18]; // 0xD4
    char lensflare[10]; // 0xE6
    char gall_num_gsr[13]; // 0xF0
    char dlh_title_waku_MOV[19]; // 0xFD
    char spinning_dolph_wav[19]; // 0x110
    char party_character_decide[23]; // 0x123
    char dlh_title_waku_OUT[19]; // 0x13A
    char dlh_arrow_IN[13]; // 0x14D
    char dlh_selects_IN[15]; // 0x15A
    char dlh_MSG_01_IN[14]; // 0x169
    char dlh_selects_MOV[16]; // 0x177
    char dlf_MSG_01_MOV[15]; // 0x187
    char dlh_selects_OUT[16]; // 0x196
    char dlh_MSG_01_OUT[15]; // 0x1A6
    char dlf_MSG_02_MOV[15]; // 0x1B5
    char dlh_eps_A[10]; // 0x1C4
    char dlh_hukidasi_MOV[17]; // 0x1CE
    char dlh_eps_B[10]; // 0x1DF
    char dlh_eps_C[10]; // 0x1E9
    char dlh_eps_D[10]; // 0x1F3
    char dlh_eps_E[10]; // 0x1FD
    char dlh_MSG_02_OUT[15]; // 0x207
    char dlh_hukidasi_OUT[17]; // 0x216
    char zeroBytes_227[1]; // 0x227
    char dlh_gsr[9]; // 0x228
    char tmb_gsr[9]; // 0x231
    char dlh_res_item_00_gsr[20]; // 0x23A
    char dlh_result_IN[14]; // 0x24E
    char dlh_result_MOV[15]; // 0x25C
    char dlh_result_OUT[15]; // 0x26B
    char rarePrizeSound[19]; // 0x27A
    char cookiePrizeSound[23]; // 0x28D
};

#include "iff/IffStruct.h"
#include "classes/CPangyaCommonDialogFields.h"
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
    kDolfiniWarmAnimations = 0, kDolfiniWaitResources = 1, kDolfiniTitleEnter = 2,
    kDolfiniTitleIdle = 3, kDolfiniTitleLeave = 4, kDolfiniArrowEnter = 5,
    kDolfiniControlsEnter = 6, kDolfiniWaitSpinInput = 7, kDolfiniControlsLeave = 8,
    kDolfiniWaitSpinStart = 9, kDolfiniSpinning = 10, kDolfiniFinishSpin = 11,
    kDolfiniPrizeEnter = 12, kDolfiniChooseReplayOrExit = 13, kDolfiniPrizeLeaveForReplay = 14,
    kDolfiniPrizeLeaveForExit = 15, kDolfiniConfirmExit = 16, kDolfiniSaveAfterPrize = 17,
    kDolfiniSaveAfterCancel = 18, kDolfiniInactive = 19, kDolfiniNoTicketsDialog = 20
};
enum { kDolfiniTicketItemId = 0x1A0000FE, kDolfiniItemLimit = 999, kDolfiniAuxPartKind = 0x1C,
       kDolfiniConfirmAction = 4, kDolfiniCancelAction = 6, kDolfiniSpinAction = 15,
       kDolfiniReplayElement = 2, kDolfiniExitElement = 3, kDolfiniPrizeImageElement = 4,
       kDolfiniPrizeTextElement = 5, kDolfiniGalleryImageElement = 6, kDolfiniNoTicketsElement = 7,
       kDolfiniTicketNumberEntry = 25, kDolfiniPrizeNumberEntry = 30, kDolfiniTitleGroup = 0x3F0,
       kDolfiniPrizeTextColor = 0x764717FF, kDolfiniFadeFrames = 15,
       kDolfiniInventoryReward = 0, kDolfiniRarePercentBase = 100,
       kDolfiniRandomDraws = 10, kDolfiniEquippedItemSlots = 8,
       kDolfiniTextureVertexFormat = 0x11F005, kDolfiniTriangleStrip = 0x98,
       kDolfiniQuadVertices = 4, kDolfiniQuadVertexBytes = 0x18 };
struct DolfiniInventoryView {
    char padding[0x174];
    short itemCounts[13];
    unsigned char auxiliaryParts[1]; // address-only start of the saved ownership flags
};

#include "classes/PlayerRecordOwner8030E168.h"
#define AsPlayerRecordOwner8030E168(p) ((PlayerRecordOwner8030E168*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Translation unit 0x8030EE64-0x80314290 (15 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CGuruguru.h"

#define AsCGuruguru(p) ((CGuruguru*)(p))
struct fn_8030EE64_String {
    unsigned int flags;
    unsigned int unused;
    char* text;
};

extern "C" {


/* ---- typed declarations for CRoundDolMain_UpdateStateMachine and the rest of the unit ---- */
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void* __register_global_object(void*, void*, void*);
void* __nw__FUl(unsigned int);
void CBpCtrl_CreateEntry(void*, const char*, void*, int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void CRoundDol3D_LoadSceneResources(void*);
void SharedFilePointerVector_InitReserved1024(void*);
void SharedFilePointerVector_Destroy(void*);
void NString_AssignCString(void*, const char*);
void appFreeUni__FPv(void*);
void CNowReloading_dtor(void*);
float SoundValuePair_GetSelectedValue(void*, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void SoundManager_SetBufferVolume(void*, float, float);
void CRoundDol3D_UpdateAnimationAndCamera(void*);
int CRoundDol3D_GetState(void*);
void CRoundDol3D_SetAnimationByIndex(void*, int);
int CBpCtrl_PollAllEntriesLoaded(void*);
int Layout_PollLoadComplete(void*);
int FileList_GetLoadedBufferByName(void*, const char*); // Retained name: returns a borrowed loaded-buffer address.
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void FadeState_StartFadeOut(void*, int);
void CNowReloading_Init(void*);
int LoadingIndicator_IsAbsentOrState3(void*);
void CB2dPlayerHandle_RequestPendingScript(void*);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetOpacity(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void CRoundDol3D_StartGroupPlayback(void*, int);
void CRoundDol3D_SetCameraTargetOverrideEnabled(void*, int);
void CFontNumDraw_RegisterEntries(void*, const char*, int, int, const char*);
void UITextTable_SetEntryResourceName(void*, int, const char*);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void CRoundDol3D_SetupModelShadows(void*);
void Layout_SetElementEnabled(void*, int, int);
void CProjectG_SetExtensionProperty(void*, int);
void Sound_SetGurudolRoundVolume(void*);
int BpObject_IsCurrentScriptComplete(void*);
int Input_IsActionPressed(void*, int, int);
int ItemInventory_FindItemIndex(void*, int);
void* __ct__19CPangyaCommonDialogFPvi(void*, int, int);
void CPangyaCommonDialog_SetText(void*, const char*);
int Input_IsActionButtonDown(void*, int, int);
void CRoundDol3D_SetCapsuleShadowEnabled(void*, int);
void CRoundDol3D_SetDolphinShadowEnabled(void*, int);
void CRoundDol3D_DecelerateRotation(void*);
int CRoundDol3D_UpdateRotationFromPadMotion(void*);
void CRoundDol3D_ResetRotation(void*);
int CRoundDol3D_GetSpeedLevel(void*);
void BpTextureBinding_ReleaseSources(void*);
void Text_SetPosition(void*, float*);
void CMsgDraw_ResetTextState(void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void CMsgDraw_SetColor(void*, int);
int wcslen(const char*);
void WString_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void* MessageTable_FindTextByName(void*, void*);
void* TextResource_FindNamedEntry(void*, void*);
void Text_SetContent(void*, const char*);
int PrizeGallery_FindMessageRecordOffset(void*, int); // PrizeGallery_FindMessageRecordOffset.
int PrizeGallery_GetImageId(void*, int); // PrizeGallery_GetImageId.
int SharedFileTable_GetMappedValueAt(void*, int); // GalleryImage_FindRecordOffset.
void* Table_ResolveDataOffset(void*, int);
void* OffsetStringNode_GetText(void*);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void* ItemInventory_FindDataWithHairStyleFallback(void*, int);
void* ItemInventory_FindDataById(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void String_ConcatCStringAndString(void*, const char*, void*);
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
void* CFlashMemoryMenuSave_Create(void);
void UIRender_SetDefaultState(void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void CRoundDolMain_RenderShadowPass(void*);
void CRoundDol3D_DrawModelsAndEffects(void*);
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
extern char roundDolGalleryQuadTemplate[]; // roundDolGalleryQuadTemplate.
extern char roundDolPrizeQuadTemplate[]; // roundDolPrizeQuadTemplate.
extern char s_roundDolDrawBpDestructorRecord[]; // s_roundDolDrawBpDestructorRecord.
extern CSharedDoc* g_sharedDoc;
extern signed char s_saveDataInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern char s_bpCtrlHandle[8];
extern signed char s_bpCtrlInitialized;
extern int kRoundDolItemTextInitialXBits; // kRoundDolItemTextInitialXBits.
extern int kRoundDolItemTextInitialYBits; // kRoundDolItemTextInitialYBits.
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
void CBpCtrlHandle_ReleaseEntry(void*);
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
        if (data) { size -= size; appFreeUni__FPv(data); }
    }
};

struct fn_8030F460_Allocator { char value; };

struct fn_8030F460_Dialog {
    char storage[0x44];
    // Construct the owned common dialog with its original kind/count arguments.
    fn_8030F460_Dialog(int kind, int count) { __ct__19CPangyaCommonDialogFPvi(this, kind, count); }
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
    ~fn_8030F460_String() { if (flags >> 31) appFreeUni__FPv(text); }
    // Read the inline or heap string length (wide-string callers use code units).
    unsigned int size() const {
        return !(flags >> 31) ? (*(const unsigned char*)this & 0x7F) : length;
    }
    // Borrow the current narrow-string character storage.
    char* data() { return !(flags >> 31) ? (char*)this + 1 : text; }
    // Copy inline words or replace the current content with a deep string copy.
    void assign(fn_8030F460_String& other) {
        unsigned int isLong = flags >> 31;
        if (!isLong && !(other.flags >> 31)) {
            flags = other.flags; length = other.length; text = other.text;
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

// Overlay of the session object: entry IDs name objects owned by the shared BP
// registry. roundDol3D, image wrappers, dialog/saveMenu and resource wrappers
// are session-held objects; table data returned from the file list is borrowed.
// savedPlayerRecord is the session's working copy, committed when exiting through save.
struct DolfiniSessionState {
    void* titleEntryId;
    void* arrowEntryId;
    void* selectionEntryId;
    void* messageEntryId;
    void* resultEntryId;
    void* speedEffectEntryId;
    void* balloonEntryId;
    void* introEntryId;
    int state;
    int speedLevel;
    int peakSpeedLevel;
    int preloadAnimationIndex;
    int reserved30;
    int spinInputArmed;
    void* roundDol3D;
    void* numberDraw;
    void* rewardTable;
    void* galleryImageTable;
    void* prizeGalleryTable;
    int rewardKind; // +4C: integer reward kind
    fn_8030F460_Text* prizeImage;
    fn_8030F460_Text* galleryImage;
    char savedPlayerRecord[0x828];
    void* dialog;
    void* layout;
    void* messageResource;
    void* textFactory;
    void* messageDraw;
    void* saveMenu;
    int resourcesLoaded;
};

// Publish the BP script controller and register its destructor once.
static inline void EnsureBpController(char* base) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base + IFF_OFFSET(DolfiniModuleStorage, bpDestructorRecord)));
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
            __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (base + IFF_OFFSET(DolfiniModuleStorage, sharedFileDestructorRecord)));
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
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (base + IFF_OFFSET(DolfiniModuleStorage, fadeDestructorRecord)));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}

// Publish the shared input manager and register its destructor once.
static inline void EnsureInputManager(char* base) {
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, base + IFF_OFFSET(DolfiniModuleStorage, inputDestructorRecord));
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
}

// Publish save data, constructing it once; the base argument is unused.
static inline void EnsureSaveData(char* base) {
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
}

// Publish the shared active-script handle and its destructor registration.
static inline void EnsureBpHandle(char* base) {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            *(int*)(s_bpCtrlHandle + 4) = -1;
            *(int*)s_bpCtrlHandle = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (base + IFF_OFFSET(DolfiniModuleStorage, bpHandleDestructorRecord)));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
}

// Publish the loading indicator and register its destructor once.
static inline void EnsureLoadingIndicator(char* base) {
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            CNowReloading_Init(s_nowReloading);
            __register_global_object(s_nowReloading, (void*)CNowReloading_dtor, (base + IFF_OFFSET(DolfiniModuleStorage, loadingDestructorRecord)));
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = s_nowReloading;
    }
}

// Choose the scene finish animation/group for the current motion-speed tier.
static inline void SelectDolfiniFinishAnimation(DolfiniSessionState* self) {
    if (6 == self->speedLevel) {
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 0xA);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 8);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
    } else if (5 == self->speedLevel) {
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 9);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 7);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
    } else if (self->speedLevel == 4) {
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 8);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 6);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
    } else {
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 7);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 5);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
    }
}

// Hide spin controls, show the capsule/dolphin shadows, and enter the finish wait.
static inline void EnterDolfiniFinishSpin(DolfiniSessionState* self, char* base) {
    EnsureBpController(base);
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_02_OUT), 1);
    EnsureBpController(base);
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->balloonEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_OUT), 1);
    EnsureBpController(base);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), 0);
    EnsureBpController(base);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->arrowEntryId), 0);
    CRoundDol3D_SetCapsuleShadowEnabled(self->roundDol3D, 1);
    CRoundDol3D_SetDolphinShadowEnabled(self->roundDol3D, 1);
    self->state = kDolfiniFinishSpin;
}

// Requires the session constructor and LoadResources to have run. Called once per
// session tick; the save menu receives a fixed 0.033 time step. Ticket/prize changes
// update the working save copy, with SaveData_SetCurrentRecord on exit. Dialogs and
// save menus created here are deleted when they finish. The temporary activation
// vector owns its array. Most waiting states still advance the 3D scene at the end.
// Run the Dolfini prize-game session: load, accept a spin, choose/apply its prize, then replay or save and exit.
int CRoundDolMain_UpdateStateMachine(void* object) {
    char* name;
    char* base = s_roundDolInputDestructorRecord;
    DolfiniSessionState* self = (DolfiniSessionState*)object;
    fn_8030F460_Coll arr;

    // Phase 1: only loading and prize/result states advance the pointer layout.
    switch (self->state) {
    // These loading/prize states continue ticking the layout while state work is pending.
    case kDolfiniWaitResources: case kDolfiniPrizeEnter: case kDolfiniChooseReplayOrExit: case kDolfiniPrizeLeaveForReplay: case kDolfiniPrizeLeaveForExit: case kDolfiniSaveAfterPrize:
        Layout_Update(self->layout);
        break;
    }

    // Phase 2: run one state. Waiting breaks reach the shared 3D update below.
    switch (self->state) { fn_8030EE64_String s; int ticketInventoryIndex; int selectedRewardId;
    // State 0: mute temporarily and warm scene animations 1..8, then await resources.
    case kDolfiniWarmAnimations:
        s_roundDolSavedBufferVolumeStorage = SoundValuePair_GetSelectedValue(g_soundManager, 0);
        SoundManager_SetBufferVolume(g_soundManager, kFloat_0_RoundDol, kFloat_0_RoundDol);
        // Warm up each animation (original loop at 0x8030F50C).
        for (;;) {
            CRoundDol3D_UpdateAnimationAndCamera(self->roundDol3D);
            if (CRoundDol3D_GetState(self->roundDol3D)) continue;
            self->preloadAnimationIndex = (1 + self->preloadAnimationIndex);
            if (self->preloadAnimationIndex <= 8) {
                CRoundDol3D_SetAnimationByIndex(self->roundDol3D, self->preloadAnimationIndex);
            } else {
                break;
            }
        }
        self->state = kDolfiniWaitResources;
        break;

    // Wait for every resource; then wrap borrowed tables and create message drawing.
    // These loading/prize states continue ticking the layout while state work is pending.
    case kDolfiniWaitResources:
        EnsureBpController(base);
        if (!CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0())) break;
        if (!Layout_PollLoadComplete(self->layout)) break;
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings)) break;
        EnsureSharedFileList(base);
        {
            void* resource = Sing_805DC680();
            {
                fn_8030F460_String s;
                NString_AssignCString(&s, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
                strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))), !(s.flags >> 31) ? (char*)&s + 1 : s.text);
                name = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
            }
            if (!FileList_GetLoadedBufferByName(resource, name)) break;
        }
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile))) break;
        EnsureSharedFileList(base);
        if (!FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile))) break;
        self->resourcesLoaded = 1;
        EnsureFadeOverlay(base);
        FadeState_StartFadeOut(Sing_805DC608(), kDolfiniFadeFrames);
        EnsureLoadingIndicator(base);
        {
            void* p = Sing_805DC610();
            if (p && LoadingIndicator_IsAbsentOrState3(p)) CB2dPlayerHandle_RequestPendingScript(p);
        }
        SoundManager_SetBufferVolume(g_soundManager, s_roundDolSavedBufferVolumeStorage, kFloat_0_RoundDol);
        self->state = kDolfiniTitleEnter;
        EnsureBpController(base);
        UIGroup_SetOpacity(UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), kDolfiniTitleGroup), 0);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_IN), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->introEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, lensflare), 1);
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 1);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 0);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
        EnsureBpController(base);
        CFontNumDraw_RegisterEntries(self->numberDraw, roundDolPrizeNumericEntries, 2,
            (int)IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        UITextTable_SetEntryResourceName(self->numberDraw, kDolfiniTicketNumberEntry, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        UITextTable_SetEntryResourceName(self->numberDraw, kDolfiniPrizeNumberEntry, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, gall_num_gsr));
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings);
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v) p[0] = v;
                p[1] = __vt__9CGuruguru;
            }
            self->rewardTable = p;
        }
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(Sing_805DC680(), (roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile)));
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v) p[0] = v;
                p[1] = __vt__14CGallery_image;
            }
            self->galleryImageTable = p;
        }
        EnsureSharedFileList(base);
        {
            void* v = (void*)FileList_GetLoadedBufferByName(Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile));
            void** p = (void**)__nw__FUl(8);
            if (p) {
                p[0] = 0;
                if (v) p[0] = v;
                p[1] = __vt__14CPrize_gallery;
            }
            self->prizeGalleryTable = p;
        }
        EnsureSharedFileList(base);
        {
            void* resource = Sing_805DC680();
            {
                fn_8030F460_String s;
                NString_AssignCString(&s, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
                strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))), !(s.flags >> 31) ? (char*)&s + 1 : s.text);
                name = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
            }
            WFixedFont_CreateMsgEcbResource(self->textFactory, &self->messageResource, FileList_GetLoadedBufferByName(resource, name), 1);
        }
        WFixedFont_CreateAndRegisterMsgDraw(self->textFactory, &self->messageDraw, 1);
        CRoundDol3D_SetupModelShadows(self->roundDol3D);
        Layout_SetElementEnabled(self->layout, kDolfiniPrizeImageElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniGalleryImageElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniPrizeTextElement, 0);
        Layout_SetElementEnabled(self->layout, kDolfiniNoTicketsElement, 0);
        CProjectG_SetExtensionProperty(g_project, 0x74C6EDFF);
        break;

    // State 2: await both title/intro scripts, then start the idle title and music.
    case kDolfiniTitleEnter:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        Layout_SetElementEnabled(self->layout, kDolfiniReplayElement, 1);
        Layout_SetElementEnabled(self->layout, kDolfiniExitElement, 1);
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId))) break;
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->introEntryId))) break;
        self->state = kDolfiniTitleIdle;
        EnsureBpController(base);
        UIGroup_SetOpacity(UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), kDolfiniTitleGroup), 0xFF);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_MOV), 1);
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 2);
        SoundManager_RequestBgmChange(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, spinning_dolph_wav), 0, 1);
        break;

    // A confirmed title starts a paid attempt only when a ticket is available.
    // State 3: confirm checks tickets; no tickets opens a dialog, otherwise leave title.
    case kDolfiniTitleIdle:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureInputManager(base);
        if (!Input_IsActionPressed(Sing_805DC5C8(), kDolfiniConfirmAction, -1)) break;
        if (CRoundDol3D_GetState(self->roundDol3D)) break;
        Sound_PlayByName(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, party_character_decide),
            0, 0, 0, 0, kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
        ticketInventoryIndex = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
        if (((DolfiniInventoryView*)self)->itemCounts[ticketInventoryIndex] <= 0) {
            {

                ((DolfiniInventoryView*)self)->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] = 0;
            }
            {
                void* p = new fn_8030F460_Dialog(3, 0xF);
                self->dialog = p;
                CPangyaCommonDialog_SetText(p, roundDolDialogStrings);
            }
            EnsureInputManager(base);
            InputManager_ResumeCursorDrawing(Sing_805DC5C8());
            self->state = kDolfiniNoTicketsDialog;
            break;
        }
        self->state = kDolfiniTitleLeave;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_waku_OUT), 1);
        EnsureBpController(base);
        UIGroup_SetOpacity(UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId), kDolfiniTitleGroup), 0);
        break;

    // State 4: await title exit, then reset the scene and bring in the arrow.
    case kDolfiniTitleLeave:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->titleEntryId))) break;
        CRoundDol3D_StartGroupPlayback(self->roundDol3D, 3);
        CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 0);
        CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 0);
        self->state = kDolfiniArrowEnter;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->arrowEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_arrow_IN), 1);
        break;

    // State 5: await arrow entry, then bring in controls and instructions.
    case kDolfiniArrowEnter:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->arrowEntryId))) break;
        self->state = kDolfiniControlsEnter;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_IN), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_IN), 1);
        break;

    // State 6: await both entry scripts, then loop controls; decelerate each tick.
    case kDolfiniControlsEnter:
        EnsureBpController(base);
        if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId))) {
            EnsureBpController(base);
            if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId))) {
                self->state = kDolfiniWaitSpinInput;
                EnsureBpController(base);
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_MOV), 1);
                EnsureBpController(base);
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_01_MOV), 1);
            }
        }
        CRoundDol3D_DecelerateRotation(self->roundDol3D);
        break;

    // Cancellation opens an owned dialog. Spin input must first arm the input latch.
    // State 7: cancel opens confirmation; armed spin input starts control exit.
    case kDolfiniWaitSpinInput:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureInputManager(base);
        if (Input_IsActionButtonDown(Sing_805DC5C8(), kDolfiniCancelAction, -1)) {
            {
                void* p = new fn_8030F460_Dialog(2, 0xA);
                self->dialog = p;
                CPangyaCommonDialog_SetText(p, roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, confirmExit));
            }
            self->state = kDolfiniConfirmExit;
            EnsureInputManager(base);
            InputManager_ResumeCursorDrawing(Sing_805DC5C8());
            break;
        }
        EnsureInputManager(base);
        if (Input_IsActionPressed(Sing_805DC5C8(), kDolfiniSpinAction, -1)) self->spinInputArmed = 1;
        EnsureInputManager(base);
        if (Input_IsActionButtonDown(Sing_805DC5C8(), kDolfiniSpinAction, -1) && self->spinInputArmed) {
            Layout_SetElementEnabled(self->layout, kDolfiniReplayElement, 1);
            Layout_SetElementEnabled(self->layout, kDolfiniExitElement, 1);
            self->state = kDolfiniControlsLeave;
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_OUT), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_OUT), 1);
        }
        CRoundDol3D_DecelerateRotation(self->roundDol3D);
        break;

    // State 8: await both exit scripts before enabling spin-start state.
    case kDolfiniControlsLeave:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId))) break;
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId))) break;
        self->state = kDolfiniWaitSpinStart;
        break;

    // State 9: recheck instruction completion, then accept motion-driven spinning.
    case kDolfiniWaitSpinStart:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId))) break;
        self->state = kDolfiniSpinning;
        break;

    // State 10: update speed effects, test peak drop, then test stopped motion/retry.
    case kDolfiniSpinning:
        // Speed-specific animation transitions (original 0x80310A50-0x80310F40).
        switch (CRoundDol3D_GetSpeedLevel(self->roundDol3D)) {
        case 0: case 1: case 2:
            break;
        case 3:
            if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == self->speedLevel) break;
            if (self->peakSpeedLevel < CRoundDol3D_GetSpeedLevel(self->roundDol3D)) self->peakSpeedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            self->speedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            EnsureBpController(base);
            BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_02_MOV), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_A), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->balloonEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_MOV), 1);
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 2);
            break;
        case 4:
            if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == self->speedLevel) break;
            if (self->peakSpeedLevel < CRoundDol3D_GetSpeedLevel(self->roundDol3D)) self->peakSpeedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            self->speedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_B), 1);
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 3);
            break;
        case 5:
            if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == self->speedLevel) break;
            if (self->peakSpeedLevel < CRoundDol3D_GetSpeedLevel(self->roundDol3D)) self->peakSpeedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            self->speedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_C), 1);
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 4);
            break;
        case 6:
            if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == self->speedLevel) break;
            if (self->peakSpeedLevel < CRoundDol3D_GetSpeedLevel(self->roundDol3D)) self->peakSpeedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            self->speedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_D), 1);
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 5);
            break;
        case 7:
            if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == self->speedLevel) break;
            if (self->peakSpeedLevel < CRoundDol3D_GetSpeedLevel(self->roundDol3D)) self->peakSpeedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            self->speedLevel = CRoundDol3D_GetSpeedLevel(self->roundDol3D);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->speedEffectEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_E), 1);
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 6);
            break;
        default:
            CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 6);
            break;
        }
        // Peak 7 ends on the first drop; lower peaks end after a two-level drop.
        // Shared transition checks (original 0x80310F40).
        if (self->peakSpeedLevel == 7) {
            if (self->peakSpeedLevel != CRoundDol3D_GetSpeedLevel(self->roundDol3D)) {
                CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 7);
                CRoundDol3D_StartGroupPlayback(self->roundDol3D, 5);
                CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 1);
                EnterDolfiniFinishSpin(self, base);
            }
        } else if (self->peakSpeedLevel == CRoundDol3D_GetSpeedLevel(self->roundDol3D) + 2) {
            SelectDolfiniFinishAnimation(self);
            EnterDolfiniFinishSpin(self, base);
        }
        // If motion stops at peak <=2, retry the controls without consuming a ticket.
        // Both transition paths rejoin here (original 0x803113C8).
        if (CRoundDol3D_UpdateRotationFromPadMotion(self->roundDol3D)) break;
        if (2 >= self->peakSpeedLevel) {
            self->peakSpeedLevel = 0;
            self->state = kDolfiniControlsEnter;
            self->speedLevel = 0;
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_IN), 1);
            EnsureBpController(base);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_MSG_01_IN), 1);
            break;
        }
        SelectDolfiniFinishAnimation(self);
        EnterDolfiniFinishSpin(self, base);
        break;

    // Once animation and messages finish, consume exactly one ticket and apply the reward.
    // State 11: wait for scene/messages; debit ticket once, award prize and build result UI.
    case kDolfiniFinishSpin:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        EnsureBpController(base);
        if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId))) {
            EnsureBpController(base);
            if (BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->balloonEntryId)) &&
                !CRoundDol3D_GetState(self->roundDol3D)) {
                { fn_8030F460_String prizeResourceName;
                    fn_8030F460_String prizeResourceStem;

                    EnsureBpHandle(base);
                    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), 0x13);
                    ticketInventoryIndex = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
                    --((DolfiniInventoryView*)self)->itemCounts[ticketInventoryIndex];
                    selectedRewardId = Dolfini_SelectAndApplyWeightedReward((struct DolfiniRewardState*)self);
                    // Build image/text resources for inventory kind 0 or gallery kind !=0.
                    if (self->rewardKind == 0) {
                        char* p = ((IffItemCommon*)ItemInventory_FindDataWithHairStyleFallback(AsCSharedDoc(g_sharedDoc)->itemInventory, selectedRewardId))->icon;
                        unsigned int count = prizeResourceStem.size();
                        fn_8030F460_Allocator allocator;
                        fn_8030F460_Allocator copy = allocator;
                        String_ReplaceRange(&prizeResourceStem, 0, count, p, p + strlen(p), (char*)&copy);
                        {
                            fn_8030F460_String t3(0);
                            fn_8030F460_String t4(0);
                            String_ConcatCStringAndString(&t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, zeroBytes_227), &prizeResourceStem);
                            String_ConstructConcatCStr(&t4, &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_gsr));
                            prizeResourceName.assign(t4);
                        }
                        BpTextureBinding_ReleaseSources(self->prizeImage);
                        self->prizeImage->set(prizeResourceName.data());
                        { float textPosition[2]; fn_8030F460_String wide;

                            *(int*)&textPosition[0] = kRoundDolItemTextInitialXBits;
                            *(int*)&textPosition[1] = kRoundDolItemTextInitialYBits;
                            Layout_GetElementPosition(self->layout, kDolfiniPrizeTextElement, textPosition);
                            textPosition[0] = kFloat_0_RoundDol;
                            Text_SetPosition(self->messageDraw, textPosition);
                            CMsgDraw_ResetTextState(self->messageDraw);
                            CMsgDraw_ClearFormattingAndRevealAll(self->messageDraw);
                            CMsgDraw_SetColor(self->messageDraw, kDolfiniPrizeTextColor);
                            {
                                char* q = roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, prizeMessagePrefix);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;
                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, q, q + wcslen(q) * 2, (char*)&copy);
                            }
                            { fn_8030F460_Allocator allocator;
                                char* p = (char*)MessageTable_FindTextByName(self->messageResource, ItemInventory_FindDataById(AsCSharedDoc(g_sharedDoc)->itemInventory, selectedRewardId));
                                unsigned int count = wide.size();

                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, p, p + wcslen(p) * 2, (char*)&copy);
                            }
                            Text_SetContent(self->messageDraw, !(wide.flags >> 31) ? (char*)&wide + 2 : wide.text);
                            fn_8030F460_Allocator allocator;
                            fn_8030F460_Allocator copy = allocator;
                            WString_ReplaceRange(&wide, 0, -1, 0, 0, (char*)&copy);
                        }
                    } else { fn_8030F460_Allocator allocator;
                        unsigned int galleryImageId = PrizeGallery_GetImageId(self->prizeGalleryTable, selectedRewardId);
                        void* galleryImageTable = self->galleryImageTable;
                        unsigned int imageRecordOffset = SharedFileTable_GetMappedValueAt(galleryImageTable, (short)galleryImageId);
                        void* imageRecord = Table_ResolveDataOffset(*(void**)galleryImageTable, imageRecordOffset);
                        char* imageResourceStem = imageRecord ? (char*)OffsetStringNode_GetText(imageRecord) : 0;
                        unsigned int count = prizeResourceStem.size();

                        fn_8030F460_Allocator copy = allocator;
                        String_ReplaceRange(&prizeResourceStem, 0, count, imageResourceStem, imageResourceStem + strlen(imageResourceStem), (char*)&copy);
                        {
                            fn_8030F460_String t4(0);
                            fn_8030F460_String t3(0);
                            String_ConcatCStringAndString(&t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, zeroBytes_227), &prizeResourceStem);
                            String_ConstructConcatCStr(&t4, &t3, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, tmb_gsr));
                            prizeResourceName.assign(t4);
                        }
                        __typeof__(self->galleryImage) galleryTexture = self->galleryImage; BpTextureBinding_ReleaseSources(self->prizeImage);
                        self->prizeImage->set(roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_res_item_00_gsr));
                        BpTextureBinding_ReleaseSources(galleryTexture);
                        self->galleryImage->set(prizeResourceName.data());
                        {
                            fn_8030F460_String wide;
                            float textPosition[2];
                            *(int*)&textPosition[0] = kRoundDolGalleryTextInitialXBits;
                            *(int*)&textPosition[1] = kRoundDolGalleryTextInitialYBits;
                            Layout_GetElementPosition(self->layout, kDolfiniPrizeTextElement, textPosition);
                            textPosition[0] = kFloat_0_RoundDol;
                            Text_SetPosition(self->messageDraw, textPosition);
                            CMsgDraw_ResetTextState(self->messageDraw);
                            CMsgDraw_ClearFormattingAndRevealAll(self->messageDraw);
                            CMsgDraw_SetColor(self->messageDraw, kDolfiniPrizeTextColor);
                            {
                                char* q = roundDolDialogStrings + IFF_OFFSET(DolfiniDialogPool, prizeMessagePrefix);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;
                                fn_8030F460_Allocator copy = allocator;
                                WString_ReplaceRange(&wide, count, 0, q, q + wcslen(q) * 2, (char*)&copy);
                            }
                            {

                                void* prizeGalleryTable = self->prizeGalleryTable; int messageRecordOffset = PrizeGallery_FindMessageRecordOffset(prizeGalleryTable, selectedRewardId);
                                void* messageRecord = Table_ResolveDataOffset(*(void**)prizeGalleryTable, messageRecordOffset);
                                void* messageKey = messageRecord ? OffsetStringNode_GetText(messageRecord) : 0;
                                char* messagePayload = (char*)TextResource_FindNamedEntry(self->messageResource, messageKey);
                                unsigned int count = wide.size();
                                fn_8030F460_Allocator allocator;

                                fn_8030F460_Allocator copy = allocator; WString_ReplaceRange(&wide, count, 0, messagePayload, messagePayload + wcslen(messagePayload) * 2, (char*)&copy);
                            }
                            Text_SetContent(self->messageDraw, !(wide.flags >> 31) ? (char*)&wide + 2 : wide.text);
                            fn_8030F460_Allocator allocator;
                            fn_8030F460_Allocator copy = allocator;
                            WString_ReplaceRange(&wide, 0, -1, 0, 0, (char*)&copy);
                        }
                    }
                    CRoundDol3D_SetCapsuleShadowEnabled(self->roundDol3D, 0);
                    self->state = kDolfiniPrizeEnter;
                    EnsureBpController(base);
                    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_IN), 1);
                    EnsureInputManager(base);
                    InputManager_ResumeCursorDrawing(Sing_805DC5C8());
                }
                break;
            }
        }
        CRoundDol3D_DecelerateRotation(self->roundDol3D);
        break;

    // State 12: wait for result script/image, then enable choices (disable replay at zero tickets).
    case kDolfiniPrizeEnter:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId))) break;
        if (!self->prizeImage->ready()) break;
        self->state = kDolfiniChooseReplayOrExit;
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_MOV), 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniReplayElement, 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniExitElement, 1);
        ticketInventoryIndex = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId);
        if (((DolfiniInventoryView*)self)->itemCounts[ticketInventoryIndex] > 0) break;
        {

            ((DolfiniInventoryView*)self)->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] = 0;
        }
        Layout_SetElementPlayerFlag(self->layout, kDolfiniReplayElement, 0, 5);
        Layout_SetElementEnabled(self->layout, kDolfiniNoTicketsElement, 1);
        break;

    // Exit commits the working record to save data; replay resets speed and result UI.
    // State 13: collect choices; exit publishes save copy, replay resets scene and hides result.
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
                    SaveData_SetCurrentRecord(Sing_805DC5E0(), self->savedPlayerRecord);
                    self->saveMenu = CFlashMemoryMenuSave_Create();
                    SoundManager_RequestBgmStop(g_soundManager, 1);
                }
                if (((int*)arr.data)[i] == kDolfiniReplayElement) {
                    CRoundDol3D_ResetRotation(self->roundDol3D);
                    Layout_SetElementPlayerFlagsAndState(self->layout, kDolfiniReplayElement, 0);
                    Layout_SetElementPlayerFlag(self->layout, kDolfiniExitElement, 0, 5);
                    self->state = kDolfiniPrizeLeaveForReplay;
                    self->speedLevel = 0;
                    self->peakSpeedLevel = 0;
                    CRoundDol3D_ResetRotation(self->roundDol3D);
                    CMsgDraw_ResetTextState(self->messageDraw);
                    CRoundDol3D_StartGroupPlayback(self->roundDol3D, 4);
                    CRoundDol3D_SetCameraTargetOverrideEnabled(self->roundDol3D, 0);
                    CRoundDol3D_SetAnimationByIndex(self->roundDol3D, 0);
                    EnsureBpController(base);
                    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_OUT), 1);
                }
            }
            break;
        }

    // State 14: wait for result exit; restore controls and suspend cursor drawing.
    case kDolfiniPrizeLeaveForReplay:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId))) break;
        self->state = kDolfiniWaitSpinInput;
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->arrowEntryId), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_selects_MOV), 1);
        EnsureBpController(base);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlf_MSG_01_MOV), 1);
        EnsureInputManager(base);
        InputManager_SuspendCursorDrawing(Sing_805DC5C8());
        EnsureBpHandle(base);
        CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
        CRoundDol3D_SetDolphinShadowEnabled(self->roundDol3D, 1);
        break;

    // State 15: await result exit and report completion without a save-menu tick.
    case kDolfiniPrizeLeaveForExit:
        EnsureBpController(base);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(Sing_805DC5F0(), self->resultEntryId))) break;
        return 1;

    // Dialog result 1 saves/exits, 2 returns to controls and clears the spin latch.
    // State 16: poll dialog; result 1 creates save UI, result 2 restores controls.
    case kDolfiniConfirmExit:
        Sound_SetGurudolRoundVolume(self->roundDol3D);
        if (!(*(int(**)(void*, float))(*(char**)self->dialog + 0x10))
            (self->dialog, kFloat_0_RoundDol)) break;
        if (AsCPangyaCommonDialogFields(self->dialog)->result == 1) {
            if (self->dialog) {
                delete (fn_8030F460_Virtual*)self->dialog;
                self->dialog = 0;
            }
            EnsureSaveData(base);
            SaveData_SetCurrentRecord(Sing_805DC5E0(), self->savedPlayerRecord);
            self->saveMenu = CFlashMemoryMenuSave_Create();
            SoundManager_RequestBgmStop(g_soundManager, 1);
            self->state = kDolfiniSaveAfterCancel;
            EnsureFadeOverlay(base);
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            break;
        }
        if (AsCPangyaCommonDialogFields(self->dialog)->result != 2) break;
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->selectionEntryId), 1);
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->messageEntryId), 1);
        EnsureBpController(base);
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->arrowEntryId), 1);
        self->state = kDolfiniControlsEnter;
        if (self->dialog) {
            delete (fn_8030F460_Virtual*)self->dialog;
            self->dialog = 0;
        }
        EnsureInputManager(base);
        InputManager_SuspendCursorDrawing(Sing_805DC5C8());
        self->spinInputArmed = 0;
        break;

    // Save completion closes the session; unfinished save frames keep the fade active.
    // State 17: poll owned save UI; delete it on completion, otherwise maintain fade.
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

    // State 18: same save completion contract after cancellation.
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
        if ((*(int(**)(void*, float))(*(char**)self->dialog + 0x10))
            (self->dialog, kFloat_0_RoundDol)) {
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

    // Shared frame update for CRoundDolMain_UpdateStateMachine (original entry 0x8030F460).
    CRoundDol3D_UpdateAnimationAndCamera(self->roundDol3D);
    return 0;
}


// Enqueues asynchronous resources, retaining BP entry IDs in self. The shared
// file-list owns queued files; textFactory is borrowed from the chat-message owner.
// Queue the prize-game UI/table resources and suspend pointer drawing while the scene loads.
void DolfiniSession_LoadResources(void* self) {
    char* base = s_roundDolInputDestructorRecord;
    EnsureBpController(base);
    // Phase 1: enqueue the eight BP layers in title/arrow/controls/message/result order.
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_title_b2d), self, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_02_ARO_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId1, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_01_COS_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId2, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_ingame_00_MSG_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId3, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId4, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_eps_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId5, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_hukidasi_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId6, 0);
    EnsureBpController(base);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dolph_tojyo_b2d), &AsPlayerRecordOwner8030E168(self)->bpEntryId7, 0);
    // Phase 2: load pointer layout and 3D scene, then queue message and reward tables.
    Layout_InitStandard(AsPlayerRecordOwner8030E168(self)->layout, 0, roundDolLayoutElements, 8, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, dlh_result_spd));
    CRoundDol3D_LoadSceneResources(AsPlayerRecordOwner8030E168(self)->roundDol3D);
    EnsureSharedFileList(base);
    void* cached680 = Sing_805DC680();
    fn_8030EE64_String local = {};
    NString_AssignCString(&local, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, iff_msg_eky));
    strcpy(((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename))), !(local.flags >> 31) ? (char*)&local + 1 : local.text);
    char* dst = ((base + IFF_OFFSET(DolfiniModuleStorage, messageFilename)));
    if (local.flags >> 31) appFreeUni__FPv(local.text);
    SharedFileList_CreateAndAppend(cached680, dst, 0);
    AsPlayerRecordOwner8030E168(self)->textFactory = ((CChatMsg*)g_chatMsg)->defaultFixedFont;
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(), roundDolPrizeStrings, 0);
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, galleryImageTableFile), 0);
    EnsureSharedFileList(base);
    SharedFileList_CreateAndAppend(Sing_805DC680(), roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, prizeGalleryTableFile), 0);
    // Phase 3: stop previous music and hide pointer drawing while resources load.
    SoundManager_RequestBgmStop(g_soundManager, 1);
    EnsureInputManager(base);
    InputManager_SuspendCursorDrawing(Sing_805DC5C8());
}

// Ensure the BP controller used by state-specific UI drawing exists.
static inline void EnsureDrawBpController() {
    if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
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
    CRoundDolMain_RenderShadowPass(AsPlayerRecordOwner8030E168(self)->roundDol3D);
    CRoundDol3D_DrawModelsAndEffects(AsPlayerRecordOwner8030E168(self)->roundDol3D);
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
static void fn_80312CF0_Draw(void* self, const char* vertices) {
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
void CRoundDolMain_DrawStateUI(void* self) {
    UIRender_SetDefaultState(self);
    // Skip state-specific drawing until ready; cleanup remains shared (entry 0x80312CF0).
    if (AsPlayerRecordOwner8030E168(self)->resourcesLoaded != 0) {
        unsigned int state = AsPlayerRecordOwner8030E168(self)->state;
        switch (state) {
        case kDolfiniTitleEnter: case kDolfiniTitleIdle: case kDolfiniTitleLeave:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId0);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId7);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId0);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId7);
            break;
        case kDolfiniArrowEnter: case kDolfiniControlsEnter: case kDolfiniWaitSpinInput: case kDolfiniControlsLeave: case kDolfiniWaitSpinStart:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            break;
        case kDolfiniSpinning: case kDolfiniFinishSpin:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId6);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId5);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId6);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId5);
            break;
        // Images use layout XY/size; templates preserve Z, packed color, and UV.
        case kDolfiniPrizeEnter: case kDolfiniChooseReplayOrExit: case kDolfiniPrizeLeaveForReplay: case kDolfiniPrizeLeaveForExit: case kDolfiniSaveAfterPrize:
            CRoundDolMain_RenderShadowPass(AsPlayerRecordOwner8030E168(self)->roundDol3D);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId4);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId4);
            Layout_DrawWithOpacity(AsPlayerRecordOwner8030E168(self)->layout, 1.0f);
            if (AsPlayerRecordOwner8030E168(self)->rewardKind) {
                if (((int (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(self)->texture1))[6])(AsPlayerRecordOwner8030E168(self)->texture1)) {
                    ((void (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(self)->texture1))[5])(AsPlayerRecordOwner8030E168(self)->texture1);
                    {
                        float a[4];
                        float b[4];
                        float v[4][6];
                        Layout_GetElementPosition(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniGalleryImageElement, a);
                        Layout_GetElementSize(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniGalleryImageElement, b);
                        memcpy((char*)v, roundDolGalleryQuadTemplate, 0x60);
                        v[0][0] = a[0]; v[0][1] = a[1];
                        v[1][0] = a[0] + b[0]; v[1][1] = a[1];
                        v[2][0] = a[0]; v[2][1] = a[1] + b[1];
                        v[3][0] = a[0] + b[0]; v[3][1] = a[1] + b[1];
                        Engine_SetFlexibleVertexFormat(kDolfiniTextureVertexFormat);
                        void* buffer = GX_GetPrimitiveCount(kDolfiniQuadVertices, kDolfiniTriangleStrip);
                        DrawPrimitiveUP(kDolfiniTriangleStrip, buffer, &v[0][0], kDolfiniQuadVertexBytes);
                    }
                }
            }
            {
                if (((int (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(self)->texture0))[6])(AsPlayerRecordOwner8030E168(self)->texture0)) {
                    ((void (*)(void*))(*(void***)(AsPlayerRecordOwner8030E168(self)->texture0))[5])(AsPlayerRecordOwner8030E168(self)->texture0);
                    {
                        float a[4];
                        float b[4];
                        float v[4][6];
                        Layout_GetElementPosition(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniPrizeImageElement, a);
                        Layout_GetElementSize(AsPlayerRecordOwner8030E168(self)->layout, kDolfiniPrizeImageElement, b);
                        memcpy((char*)v, roundDolPrizeQuadTemplate, 0x60);
                        v[0][0] = a[0]; v[0][1] = a[1];
                        v[1][0] = a[0] + b[0]; v[1][1] = a[1];
                        v[2][0] = a[0]; v[2][1] = a[1] + b[1];
                        v[3][0] = a[0] + b[0]; v[3][1] = a[1] + b[1];
                        Engine_SetFlexibleVertexFormat(kDolfiniTextureVertexFormat);
                        void* buffer = GX_GetPrimitiveCount(kDolfiniQuadVertices, kDolfiniTriangleStrip);
                        DrawPrimitiveUP(kDolfiniTriangleStrip, buffer, &v[0][0], kDolfiniQuadVertexBytes);
                    }
                }
            }
            break;
        case kDolfiniConfirmExit:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            ((void (*)(void*))(*(void***)AsPlayerRecordOwner8030E168(self)->dialog)[5])(AsPlayerRecordOwner8030E168(self)->dialog);
            break;
        case kDolfiniSaveAfterCancel:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId1);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId3);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId2);
            /* original case 18 (jump table 0x804B8484 -> 0x803138B4) ends at 0x80313B10 with b to the 0x894 check: no slot-0x14 call on +0x880 */
            break;
        case kDolfiniNoTicketsDialog:
            DrawDolfiniScene(self);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId0);
            InvokeDolfiniEntryCallback(&AsPlayerRecordOwner8030E168(self)->bpEntryId7);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId0);
            DrawDolfiniEntry(&AsPlayerRecordOwner8030E168(self)->bpEntryId7);
            ((void (*)(void*))(*(void***)AsPlayerRecordOwner8030E168(self)->dialog)[5])(AsPlayerRecordOwner8030E168(self)->dialog);
            break;
        case kDolfiniWarmAnimations: case kDolfiniWaitResources: case kDolfiniInactive:

            break;
        }
    }
    if (AsPlayerRecordOwner8030E168(self)->saveMenu) MPBase1_DrawIfInitialized(AsPlayerRecordOwner8030E168(self)->saveMenu);
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
int CRoundDol3D_GetSpeedLevel(void*);
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
static DolfiniRewardEntry* fn_80313CE8_GetItem(DolfiniRewardState* self, int index) {
	return Table_GetEntry(self->rewardTable->list, index);
}

// Return the loaded reward-table entry count, or zero while its list is absent.
static int fn_80313CE8_Count(DolfiniRewardState* self) {
	DolfiniRewardList* list = self->rewardTable->list;
	return list ? list->count : 0;
}

// Draw the ordinary-pool weight from TEN random values; rare draws use ONE.
static inline int DrawOrdinaryDolfiniWeight(int totalWeight) {
    int sum = 0;
    for (int draw = 0; draw < kDolfiniRandomDraws; ++draw) sum += rand();
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
	if (CRoundDol3D_GetSpeedLevel(self->roundDol3D) == 6) divisor = 2;
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
			if (0 != enabled && 0 != isRarePool) totalWeight += weight;
			i += 1;
		}
		int remainingWeight = rand() % totalWeight;
		int rareTableOrdinal = 0;
		for (;;) {
			if (rareTableOrdinal == fn_80313CE8_Count(self)) rareTableOrdinal = 0;
			DolfiniRewardEntry* item = Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, rareTableOrdinal);

			short weight = item->weight;
			short isRarePool = item->rare;
			if ((item->enabled) != 0 && 0 != isRarePool) remainingWeight -= weight;
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
		Sound_PlayByName(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, rarePrizeSound),
			0, 0, 0, 0, kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
	} else {
		int i = 0;
		while (i < (self->rewardTable->list ? self->rewardTable->list->count : 0)) {
			DolfiniRewardEntry* item = Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, i);

			short weight = item->weight;
			short isRarePool = item->rare;
			if ((item->enabled) != 0 && !isRarePool) totalWeight += weight;
			i = (i + 1);
		}
		// Ordinary draws sum ten rand() results; rare draws use one. Preserve the
		// number/order of calls when tuning weights to retain RNG behavior.
		int ordinaryDraw = DrawOrdinaryDolfiniWeight(totalWeight);

		// The ordinary scan ordinal and the eventual prize quantity have separate roles.
		ordinaryTableOrdinal = 0; int remainingWeight = ordinaryDraw;
		for (;;) {
			if (ordinaryTableOrdinal == fn_80313CE8_Count(self)) ordinaryTableOrdinal = 0;
			DolfiniRewardEntry* item = Table_GetEntry(*(DolfiniRewardList* volatile*)&self->rewardTable->list, ordinaryTableOrdinal);

			short weight = item->weight;
			short isRarePool = item->rare;
			if (0 != (item->enabled) && isRarePool == 0) remainingWeight -= weight;
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
		Sound_PlayByName(g_soundManager, roundDolPrizeStrings + IFF_OFFSET(DolfiniPrizeStringPool, cookiePrizeSound),
			0, 0, 0, 0, kFloat_0_5_RoundDol, kFloat_200_RoundMenu, kFloat_1_RoundDolSound);
	}
	// Phase 2: inventory rewards set auxiliary ownership, or add quantities with a
	// 999-item cap that includes all matching equipped slots. Gallery rewards skip this.
	if (rewardKind == kDolfiniInventoryReward) {
		if ((unsigned int)rewardId >> 26 == kDolfiniAuxPartKind) {

			((DolfiniInventoryView*)self)->auxiliaryParts[ItemInventory_FindAuxPartIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)] = 1;
		} else {
			int equippedCount = 0;
			for (int i = 0; i < kDolfiniEquippedItemSlots; ++i) {
				if (self->equippedItemIndices[i] == ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)) ++equippedCount;
			}
			int total = equippedCount + self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)];
			int countAfterPrize = prizeQuantity + total;
			if (countAfterPrize >= kDolfiniItemLimit) {
				self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)] = (short)(kDolfiniItemLimit - equippedCount);
			} else {
				self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)rewardId)] += prizeQuantity;
			}
		}
	}
	// Phase 3: normalize the remaining ticket count to [0,999] and refresh the counter.
	if (self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] <= 0) {
		self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] = 0;
	}
	if (self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] > kDolfiniItemLimit) {
		self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)] = kDolfiniItemLimit;
	}
	UITextTable_SetInteger(self->numberDraw, kDolfiniTicketNumberEntry, self->itemCounts[ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, kDolfiniTicketItemId)]);
	return rewardId;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_8() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_7() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_6() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_5() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_4() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_3() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_2() {
	return 1;
}

// Default table callback reports success without additional work.
int CGuruguru_vfunc_1() {
	return 1;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CGuruguru_vfunc_0(void* self, int flag) {
	if (self && flag > 0) __dl__FPv(self);
	return self;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CGallery_image_vfunc_0(void* self, int flag) {
	if (self && flag > 0) __dl__FPv(self);
	return self;
}

// Optionally delete the table wrapper; borrowed table data is not released here.
void* CPrize_gallery_vfunc_0(void* self, int flag) {
	if (self && flag > 0) __dl__FPv(self);
	return self;
}

} // extern "C"

