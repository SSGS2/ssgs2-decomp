#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Byte offsets into the existing packed string pool; no new string storage.
enum DartsMessageOffset {
    kDartsCenterTag = 0x2E,
    kDartsConditionMessage = 0x3C,
    kDartsNonzeroClearTypeMessage = 0x72,
    kDartsRightTag = 0xA8,
    kDartsZeroClearTypeMessage = 0xB4,
    kDartsAlignmentTagOpen = 0xEA,
    kDartsTagClose = 0xF6,
    kDartsDictionaryTagOpen = 0xFA,
};

// Byte offsets into the existing packed string pool; no new string storage.
enum DartsStringOffset {
    kDartsAnimationResource = 0x14,
    kDartsInitialScript = 0x28,
    kDartsNumberFont = 0x33,
    kDartsTargetNumberGroup = 0x40,
};

// Byte offsets into the existing packed string pool; no new string storage.
enum SaveLoadStringOffset {
    kSaveLoadLayout = 0,
    kSaveLoadAnimation = 0xD,
    kSaveLoadTitleAnimation = 0x1A,
    kSaveLoadTextureArchive = 0x2F,
};

#include "classes/SavePlayerRecord.h"

struct ChatMessageResourceView { char padding[0x10]; void* textFactory; };
enum DartsDialogState { kDartsIdle = 0, kDartsLoading = 1, kDartsWaitScript = 2, kDartsFading = 3 };
enum DartsFadeState { kDartsFadeComplete = 0, kDartsFadeIn = 1, kDartsHold = 2, kDartsFadeOut = 3 };
// Text uses packed 0xRRGGBBAA; BP group opacity uses 0..128 for 0..100%.
enum DartsOpacity { kDartsWhiteRgb = -256, kDartsAlphaMask = 255, kDartsGroupOpaque = 128 };
enum SaveLoadUiConstants { kSavePlayerSlots = 4, kNoSaveSlot = -1, kLoadResultNone = 0, kLoadResultCancelled = 1, kMenuActivationAction = 5 };
#include "classes/CSharedDoc.h"
// Translation unit 0x803371A0-0x80338838 (13 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadDataMenu.h"
#include "classes/CDartsDialog.h"
// CDartsDialog_Ctor (0x803371A0): BP entry ID +0x10 and message resource +0x20.
// CLoadDataMenu_Ctor (0x80338130): entry IDs +0x20/+0x20EC, message +0x30,
// and the player-record range ends at selectedSlot (+0x20D8).

#define AsCDartsDialog(p) ((CDartsDialog*)(p))

#define AsCLoadDataMenu(p) ((CLoadDataMenu*)(p))

extern "C" {
extern char s_fadeOverlayDtorRegistration_80338838[];


extern float s_saveLoadMenuFullTurnAngle; // Module angle storage initialized to one full turn.
extern float s_dartsDialogUpdateFullTurnAngle; // Module angle storage initialized to one full turn.
extern float s_dartsDialogDrawFullTurnAngle; // Module angle storage initialized to one full turn.
extern float kFloat_6_2831855;
extern float kFloat_1_DartsDialogOpacity; // 1: kFloat_1_DartsDialogOpacity.
extern const float kFloat_0_DartsDialog;
extern float kFloat_0_5_FadeTransition;
extern float kFloat_255_DartsDialog;
extern float kDartsDialogHoldDuration; // 5: kDartsDialogHoldDuration.
extern char* dartsDialogWindowGroupNames[];
extern char dartsDialogStrings[];
extern char dartsDialogNumberEntryNames[]; // Numeric entry name for the darts target group.
extern char dartsDialogMessageStrings[];
extern int g_dartsPointGroupName;
extern int g_dartsDialogOverGroupName;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_dartsDialogInputDtorRegistration[]; // Destructor registration storage for dartsDialogInputDtorRegistration.
extern char s_dartsDialogBpCtrlDtorRegistration[]; // Destructor registration storage for dartsDialogBpCtrlDtorRegistration.
extern char s_dartsDialogNumberRegistryDtorRegistration[]; // Destructor registration storage for dartsDialogNumberRegistryDtorRegistration.

extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
static char s_bpCtrlDtorRecord_SaveLoad[12];
extern char s_bpCtrlDtorRecord_SaveLoad[];
static char s_bpCtrlHandleDtorRegistration_DartsDialog[12];
#pragma force_active on
static char reservedRecords[24];
#pragma force_active off
static char record30[12];
static char record3C[12];
#pragma force_active on
static char record48[12];
#pragma force_active off

void CPackTextureResource_PollLoad(void*);
void Layout_Update(void*);
void Layout_DrawWithOpacity(void*, float);
void BpObject_Draw(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void UITextTable_Draw(void*);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void* CBpCtrl_CreateEntry(void*, const char*, void**, int);
void* IdMap_Lookup(void*, void*);
void* PointerVector_InitReserved(void*);
void* CFontNumDraw_CreateInVector(void*);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void**, int);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*); // PointerPairList_Dtor.
void* PointerPairList_RemoveByKey(void*, void*);
void UITextTable_ClearEntries(void*);
void* PointerArray_DeleteValue(void*, void*);
void CBpCtrl_DeleteEntryById(void*, int);
void Layout_Destroy(void*, int);
int Layout_PollLoadComplete(void*);
int BpObject_PollLoad(void*);
void Layout_SetElementEnabled(void*, int, int);
void BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetOpacity(void*, int);
void CFontNumDraw_RegisterEntries(void*, void*, int, void*, const char*);
void UITextTable_SetInteger(void*, int, void*);
int BpObject_IsCurrentScriptComplete(void*);
int CDartsDialog_UpdateFadeSequence(void*, float);
void CDartsDialog_SetTaggedTextAtLayoutElement(void*, int, const char*, const char*, int);
void PointerVector_DestroyStorage(void*);
void MessageDrawPairList_Dtor(void*);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void CBpCtrlHandle_Dtor(void*);
void WString_ReplaceRange(void*, int, int, const char*, const char*, char*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Layout_GetElementPosition(void*, int, void*);
void Layout_GetElementSize(void*, int, void*); // Layout_GetElementSize.
void Text_SetPosition(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
void Text_SetContent(void*, void*);
void CMsgDraw_SetAllGlyphColors(void*, int);
int InputManager_IsAnyMenuAdvanceActionActive(void*);
void InputManager_Init(void*);
void InputManager_Dtor(void);
void appFreeUni__FPv(void*);
void* MPBase1_Ctor(void*);
void SaveData_InitPlayerRecord(void*);
void* ResourceTable_FindEntryByName(void*, const char*, unsigned int*);
void* CPackTextureResource_Init(void*, const char*, unsigned int, void*);
void* CBpTex_Init(void*);
void Allocation_Dtor(void*);
extern char __vt__13CLoadDataMenu[];
extern char saveLoadMenuStrings[];
extern char s_bpCtrl[];
extern char __vt__12CDartsDialog[];
extern char s_messageDrawPairList[];
extern char s_fontNumDrawRegistry[];
extern char coursePreviewTextureNames[];
extern void* g_sharedDoc;
extern int g_chatMsg;
extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern signed char s_pointerVectorInitialized;
extern signed char s_bpCtrlInitialized;

struct ParseResult {
    union {
        unsigned int flags;
        unsigned char tag;
    };
    unsigned int value;
    void* allocation;
    // Release only heap-tagged UTF-16 string storage; inline text belongs here.
    ~ParseResult() {
        if (flags >> 31) appFreeUni__FPv(allocation);
    }
};

struct TexObj {
    virtual void v0();
    virtual void v1(void*);
};

#pragma opt_common_subs off
// Allocate the save/load layout, initialize four player records, and request UI resources.
void* CLoadDataMenu_Ctor(void* self) {
    // Phase 1: initialize base, layout, owned record storage and invalid entry
    // IDs. Phase 2: request layout/BP entries, register text/number renderers.
    // Phase 3: wrap the borrowed archive entry, create BP texture and select
    // the shared document's preview. Returns self; caller supplies storage.
    char* names = s_bpCtrlDtorRecord_SaveLoad;
    MPBase1_Ctor(self);
    *(void**)self = __vt__13CLoadDataMenu;
    void* p = __nw__FUl(0x5C);
    if (p) Layout_Init(p, kMenuActivationAction);
    AsCLoadDataMenu(self)->layout = p;
    (*(int*)&AsCLoadDataMenu(self)->bpObject) = 0;
    AsCLoadDataMenu(self)->bpEntryId = -1;
    AsCLoadDataMenu(self)->selectionDelayFrames = -1;
    (*(char*)&AsCLoadDataMenu(self)->finished) = 0;
    AsCLoadDataMenu(self)->textFactory = 0;
    (*(int*)&AsCLoadDataMenu(self)->messageDraw) = 0;
    (*(int*)&AsCLoadDataMenu(self)->textTable) = 0;
    {
        SavePlayerRecord* end;
        SavePlayerRecord* item;
        item = (SavePlayerRecord*)&AsCLoadDataMenu(self)->slotUsed;
        end = (SavePlayerRecord*)&AsCLoadDataMenu(self)->selectedSlot;
        do {
            SaveData_InitPlayerRecord(item);
            ++item;
        } while (item < end);
    }
    AsCLoadDataMenu(self)->selectedSlot = kNoSaveSlot;
    AsCLoadDataMenu(self)->result = 0;
    (*(int*)&AsCLoadDataMenu(self)->packTextureResource) = 0;
    (*(int*)&AsCLoadDataMenu(self)->bpTexture) = 0;
    AsCLoadDataMenu(self)->savedScriptIndex = -1;
    AsCLoadDataMenu(self)->secondaryBpEntryId = -1;
    (*(int*)&AsCLoadDataMenu(self)->deleteConfirmationDialog) = 0;
    Memory_Clear(&AsCLoadDataMenu(self)->slotUsed, sizeof(SavePlayerRecord) * kSavePlayerSlots);
    Layout_LoadExtended(AsCLoadDataMenu(self)->layout, 0, saveLoadMenuStrings + kSaveLoadLayout);
    // Phase 2: controller owns BP objects; this menu retains their IDs and
    // borrowed object pointers. Registry membership is removed in the dtor.
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), saveLoadMenuStrings + kSaveLoadAnimation, (void**)&AsCLoadDataMenu(self)->bpEntryId, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    AsCLoadDataMenu(self)->bpObject = IdMap_Lookup(Sing_805DC5F0(), (void*)AsCLoadDataMenu(self)->bpEntryId);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), saveLoadMenuStrings + kSaveLoadTitleAnimation, (void**)&AsCLoadDataMenu(self)->secondaryBpEntryId, 0);
    void* mgr = ((ChatMessageResourceView*)g_chatMsg)->textFactory;
    (*(void**)&AsCLoadDataMenu(self)->textFactory) = mgr;
    WFixedFont_CreateAndRegisterMsgDraw(mgr, &AsCLoadDataMenu(self)->messageDraw, 1);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (names + 0x30));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    void* list = CFontNumDraw_CreateInVector(Sing_805DC830());
    void* mgr620 = Sing_805DC620();
    AsCLoadDataMenu(self)->textTable = list;
    unsigned int size = 0;
    if (!mgr620) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (names + 0x48));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* result = ResourceTable_FindEntryByName(Sing_805DC620(), saveLoadMenuStrings + kSaveLoadTextureArchive, (unsigned int*)&size);
    // Phase 3: own wrappers, not the resource table's archive entry itself.
    p = __nw__FUl(0x20);
    if (p) {
        try {
            CPackTextureResource_Init(p, saveLoadMenuStrings + kSaveLoadTextureArchive, size, result);
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    AsCLoadDataMenu(self)->packTextureResource = p;
    p = __nw__FUl(0x28);
    if (p) CBpTex_Init(p);
    AsCLoadDataMenu(self)->bpTexture = p;
    int index = (*(int*)&((CSharedDoc*)g_sharedDoc)->menuTextureIndex);
    void* tex = ((void**)coursePreviewTextureNames)[index];
    ((TexObj*)p)->v1(tex);
    return self;
}
#pragma opt_common_subs reset

#pragma opt_lifetimes off
// Wait for resources and the opening script, then advance the timed darts dialog.
int CDartsDialog_Update(void* self, float delta) {
    // Outer state: loading(1) -> wait script(2) -> fading(3) -> idle(0).
    // The fade enum happens to share 1/2/3; these cases test OUTER state.
    // Every satisfied transition falls through and polls the next phase now.
    // Return 0 while waiting, 1 on completion/default. delta shares the fade
    // clock's units (0.5 transition, 5 hold); no unit conversion occurs here.
    Layout_Update(((CDartsDialog*)self)->layout);
    BpObject_TryInvokeCurrentCallback((*(void**)&((CDartsDialog*)self)->bpObject), 0);
    int state = ((CDartsDialog*)self)->state;
    switch (state) {
        // 0x80337640: initialization falls through to polling and fading.
    case kDartsLoading: {
            if (!Layout_PollLoadComplete(((CDartsDialog*)self)->layout)) break;
            if (!BpObject_PollLoad((*(void**)&((CDartsDialog*)self)->bpObject))) break;
            for (unsigned int i = 0; i < 4; ++i)
                Layout_SetElementEnabled(((CDartsDialog*)self)->layout, i, 0);
            BpObject_StartScriptByName((*(void**)&((CDartsDialog*)self)->bpObject), dartsDialogStrings + kDartsInitialScript, 1);
            // BP objects are borrowed from the ID registry. The shared header
            // stores bpObject as int, so casts recover its 32-bit pointer bits.
            // These arrays snapshot seven window names and two dynamic names;
            // copying them neither creates groups nor transfers ownership.
            char* dyn_names[2];
            char* names[7];
            {
                char** src = (char**)dartsDialogWindowGroupNames;
                char** dst = names;
                for (unsigned int i = 0; i < 7; ++i) {
                    *dst = *src;
                    ++src;
                    ++dst;
                }
            }
            for (unsigned int i = 0; i < 7; ++i) {
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&((CDartsDialog*)self)->bpObject), names[i]), 0);
            }
            CFontNumDraw_RegisterEntries(((CDartsDialog*)self)->textTable, &dartsDialogNumberEntryNames, 1,
            (*(void**)&((CDartsDialog*)self)->bpObject), dartsDialogStrings + kDartsNumberFont);
            UITextTable_SetInteger(((CDartsDialog*)self)->textTable, 9, ((CDartsDialog*)self)->targetValue);
            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&((CDartsDialog*)self)->bpObject), dartsDialogStrings + kDartsTargetNumberGroup), 0);
            dyn_names[0] = (char*)g_dartsPointGroupName;
            dyn_names[1] = (char*)g_dartsDialogOverGroupName;
            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&((CDartsDialog*)self)->bpObject), dyn_names[0]), 0);
            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&((CDartsDialog*)self)->bpObject), dyn_names[1]), 0);
            CDartsDialog_SetTaggedTextAtLayoutElement(self, 0, dartsDialogMessageStrings, dartsDialogMessageStrings + kDartsCenterTag, 0);
            CDartsDialog_SetTaggedTextAtLayoutElement(self, 1, dartsDialogMessageStrings + kDartsConditionMessage, 0, 1);
            if ((*(int*)&((CDartsDialog*)self)->clearType))
                CDartsDialog_SetTaggedTextAtLayoutElement(self, 3, dartsDialogMessageStrings + kDartsNonzeroClearTypeMessage, dartsDialogMessageStrings + kDartsRightTag, 3);
            else
                CDartsDialog_SetTaggedTextAtLayoutElement(self, 3, dartsDialogMessageStrings + kDartsZeroClearTypeMessage, dartsDialogMessageStrings + kDartsRightTag, 3);
            ((CDartsDialog*)self)->state = kDartsWaitScript;
        }
    case kDartsWaitScript:
        if (!BpObject_IsCurrentScriptComplete((*(void**)&((CDartsDialog*)self)->bpObject))) break;
        ((CDartsDialog*)self)->fadeState = kDartsFadeIn;
        ((CDartsDialog*)self)->fadeTime = kFloat_0_DartsDialog;
        ((CDartsDialog*)self)->state = kDartsFading;
    case kDartsFading:
        if (!CDartsDialog_UpdateFadeSequence(self, delta)) break;
        ((CDartsDialog*)self)->state = kDartsIdle;
    default:
        return 1;
    }
    return 0;
}
#pragma opt_lifetimes reset

// Fade the darts groups/text in, hold until input or timeout, then fade them out.
int CDartsDialog_UpdateFadeSequence(void* arg, float delta) {
    // Phase diagram: fade in(1, 0.5) -> hold(2, input or 5) -> fade out(3, 0.5)
    // -> complete(0). A completed phase resets time to zero, discarding overshoot,
    // and falls through to process the next phase during THIS call.
    // continue exits the do/while(0), then adds delta ONCE and returns 0.
    // The switch's break reaches return 1, skipping the time increment. This
    // distinction is why replacing continue with break changes fade timing.
    // Text slots 0,1,3 follow alpha; slot 2 is untouched. Window/point/number
    // groups use ceil(alpha/2); the over group is always hidden by this sequence.
    struct Names { char* v[7]; };
    struct Pair { const char* v[2]; };
    CDartsDialog* self = AsCDartsDialog(arg);
    int state = self->fadeState;
    do {
        switch (state) {
            // 0x803378D4: each completed phase falls through to the next.
        case kDartsFadeIn: {
                char** p;
                unsigned int i;
                int color, half, alpha;
                if (self->fadeTime < kFloat_0_5_FadeTransition) {
                    alpha = (int)(kFloat_255_DartsDialog * self->fadeTime / kFloat_0_5_FadeTransition);
                    Names names = *(Names*)dartsDialogWindowGroupNames;
                    half = ((unsigned char)alpha + 1) / 2;
                    p = names.v;
                    for (i = 0; i < 7; ++i, ++p) {
                        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), half);
                    }
                    color = kDartsWhiteRgb | (alpha & kDartsAlphaMask);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[0], color);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[1], color);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), dartsDialogStrings + kDartsTargetNumberGroup), half);
                    Pair pair = *(Pair*)&g_dartsPointGroupName;
                    pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), half);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[3], color);
                    continue;
                } else {
                    Names names = *(Names*)dartsDialogWindowGroupNames;
                    p = names.v;
                    for (i = 0; i < 7; ++i, ++p)
                        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), 128);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[0], -1);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[1], -1);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), dartsDialogStrings + kDartsTargetNumberGroup), 128);
                    Pair pair = *(Pair*)&g_dartsPointGroupName;
                    pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), 128);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[3], -1);
                }
                self->fadeState = kDartsHold;
                self->fadeTime = kFloat_0_DartsDialog;
            }
        case kDartsHold:
            if (!((int&)Sing_805DC5C8())) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_dartsDialogInputDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
            }
            if (InputManager_IsAnyMenuAdvanceActionActive((void*)((int&)Sing_805DC5C8())) || !(self->fadeTime < kDartsDialogHoldDuration)) {
                self->fadeState = kDartsFadeOut;
                self->fadeTime = kFloat_0_DartsDialog;
            } else continue;
        case kDartsFadeOut: {
                int alpha, half, color;
                char** p;
                unsigned int i;
                if (self->fadeTime < kFloat_0_5_FadeTransition) {
                    alpha = (int)(kFloat_255_DartsDialog - kFloat_255_DartsDialog * self->fadeTime / kFloat_0_5_FadeTransition);
                    Names names = *(Names*)dartsDialogWindowGroupNames;
                    half = ((unsigned char)alpha + 1) / 2;
                    p = names.v;
                    for (i = 0; i < 7; ++i, ++p)
                        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), half);
                    color = kDartsWhiteRgb | (alpha & kDartsAlphaMask);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[0], color);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[1], color);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), dartsDialogStrings + kDartsTargetNumberGroup), half);
                    Pair pair = *(Pair*)&g_dartsPointGroupName;
                    pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), half);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[3], color);
                    continue;
                } else {
                    Names names = *(Names*)dartsDialogWindowGroupNames;
                    p = names.v;
                    for (i = 0; i < 7; ++i, ++p)
                        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), 0);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[0], -256);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[1], -256);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), dartsDialogStrings + kDartsTargetNumberGroup), 0);
                    Pair pair = *(Pair*)&g_dartsPointGroupName;
                    pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), 0);
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                    CMsgDraw_SetAllGlyphColors(self->textDraws[3], -256);
                    self->fadeState = kDartsFadeComplete;
                    self->fadeTime = kFloat_0_DartsDialog;
                    break;
                }
            }
        default:
            break;
        }
        return 1;
    } while (0);
    self->fadeTime += delta;
    return 0;
}

// Initialize the load-menu module full-turn angle constant.
void SaveLoadMenu_InitFullTurnAngle() {
    s_saveLoadMenuFullTurnAngle = kFloat_6_2831855;
}

// Initialize the darts update module full-turn angle constant.
void DartsDialog_InitUpdateFullTurnAngle() {
    s_dartsDialogUpdateFullTurnAngle = kFloat_6_2831855;
}

// Initialize the darts draw module full-turn angle constant.
void DartsDialog_InitDrawFullTurnAngle() {
    s_dartsDialogDrawFullTurnAngle = kFloat_6_2831855;
}

// Poll the texture archive and update the save/load layout.
void CLoadDataMenu_PollTextureLoadAndUpdateLayout(void* self) {
    void* value = AsCLoadDataMenu(self)->packTextureResource;
    if (value != 0) {
        CPackTextureResource_PollLoad(value);
    }
    Layout_Update(AsCLoadDataMenu(self)->layout);
}

struct VP0_ { 
virtual void destroy(int);
};
// Release menu textures, text renderers, BP entries, and layout resources.
void* CLoadDataMenu_dtor(void* self, int deleting) {
    // Phase 1: unregister/clear numeric and message rendering, then destroy the
    // message draw and layout. Phase 2: delete two controller-owned BP entries
    // by ID, then own texture wrappers. Phase 3: restore saved script index;
    // free self only when deleting > 0. Returns the original (possibly freed)
    // address, following the compiler's destructor ABI; do not dereference it.
    if (self != 0) {
        *(void**)self = __vt__13CLoadDataMenu;
        UITextTable_ClearEntries(AsCLoadDataMenu(self)->textTable);
        if (AsCLoadDataMenu(self)->textTable != 0) {
            if (Sing_805DC830() == 0) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, record30);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCLoadDataMenu(self)->textTable);
        }
        if (Sing_805DC650() == 0) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, record3C);
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), AsCLoadDataMenu(self)->messageDraw);
        void* p = AsCLoadDataMenu(self)->messageDraw;
        if (p != 0) {
            if (p != 0) {
                ((VP0_*)p)->destroy(1);
            }
            AsCLoadDataMenu(self)->messageDraw = 0;
        }
        p = AsCLoadDataMenu(self)->layout;
        if (p != 0) {
            Layout_Destroy(p, 1);
            AsCLoadDataMenu(self)->layout = 0;
        }
        if (AsCLoadDataMenu(self)->bpEntryId != -1) {
            // Phase 2: IDs, not bpObject pointers, control BP object lifetime.
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCLoadDataMenu(self)->bpEntryId);
            AsCLoadDataMenu(self)->bpEntryId = -1;
        }
        if (AsCLoadDataMenu(self)->secondaryBpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCLoadDataMenu(self)->secondaryBpEntryId);
            AsCLoadDataMenu(self)->secondaryBpEntryId = -1;
        }
        p = AsCLoadDataMenu(self)->bpTexture;
        if (p != 0) {
            if (p != 0) {
                ((VP0_*)p)->destroy(1);
            }
            AsCLoadDataMenu(self)->bpTexture = 0;
        }
        p = AsCLoadDataMenu(self)->packTextureResource;
        if (p != 0) {
            if (p != 0) {
                ((VP0_*)p)->destroy(1);
            }
            AsCLoadDataMenu(self)->packTextureResource = 0;
        }
        if (Sing_805DC5F8() == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[1] = -1;
                s_bpCtrlHandle[0] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_DartsDialog);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), AsCLoadDataMenu(self)->savedScriptIndex);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Return the load result and write a slot only for a valid non-cancelled selection.
int CLoadDataMenu_GetSelectionResult(void* self, int* selectedSlotOut) {
    // Output is unchanged for pending, cancelled or invalid slot; those return
    // 1. Otherwise write 0..3 and return the stored result code unchanged.
    int result = ((CLoadDataMenu*)self)->result;
    if (result == kLoadResultNone) {
        return 1;
    }
    if (result == kLoadResultCancelled) {
        return 1;
    }
    int selectedSlot = ((CLoadDataMenu*)self)->selectedSlot;
    if (selectedSlot < 0 || selectedSlot >= kSavePlayerSlots) {
        return 1;
    }
    *selectedSlotOut = selectedSlot;
    return ((CLoadDataMenu*)self)->result;
}

// Draw the loaded darts layout, BP animation, and numeric text table.
void CDartsDialog_Draw(void* self) {
    switch (((CDartsDialog*)self)->state) {
    case kDartsLoading:
        return;
    default:
        Layout_DrawWithOpacity(((CDartsDialog*)self)->layout, kFloat_1_DartsDialogOpacity);
        BpObject_Draw((*(void**)&((CDartsDialog*)self)->bpObject));
        UITextTable_Draw(((CDartsDialog*)self)->textTable);
        return;
    }
}

#pragma use_lmw_stmw on
// Expand a message key with an optional alignment tag and place it at a layout element.
void CDartsDialog_SetTaggedTextAtLayoutElement(void* self, int textSlot, const char* messageKey, const char* alignmentTag, int layoutElement) {
    // Inputs point to UTF-16 text despite their char* ABI. Caller must supply
    // 0 <= textSlot < 4 (only the upper bound is checked), a valid layout ID,
    // and terminated keys/tags. ParseResult owns temporary heap strings.
    // Phase 1: build optional alignment markup and the dictionary lookup tag.
    if (textSlot >= 4) return;
    ParseResult expandedText;
    ParseResult dictionaryTag;
    void* textDraw = AsCDartsDialog(self)->textDraws[textSlot];
    char position[8];
    char layoutSize[8];
    char pair1[8];
    char pair2[8];
    char pair3[8];
    char pair4[8];
    char pair5[8];
    char pair6[8];
    expandedText.flags = 0;
    expandedText.value = 0;
    expandedText.allocation = 0;
    int v1;
    const char* key;
    if (alignmentTag != 0) {
        key = dartsDialogMessageStrings + kDartsAlignmentTagOpen;
        v1 =(expandedText.flags >> 31) == 0 ? (expandedText.tag & 0x7F) : expandedText.value;
        *(volatile char*)(pair1 + 0) = *(volatile char*)(pair1 + 4);
        int len = wcslen((const wchar_t*)key);
        WString_ReplaceRange(&expandedText, 0, v1, key, key + len * 2, pair1);
        int v2 = (expandedText.flags >> 31) == 0 ? (expandedText.tag & 0x7F) : expandedText.value;
        *(volatile char*)(pair2 + 0) = *(volatile char*)(pair2 + 4);
        len = wcslen((const wchar_t*)alignmentTag);
        WString_ReplaceRange(&expandedText, v2, 0, alignmentTag, alignmentTag + len * 2, pair2);
        key = dartsDialogMessageStrings + kDartsTagClose;
        int v3 = (expandedText.flags >> 31) == 0 ? (expandedText.tag & 0x7F) : expandedText.value;
        *(volatile char*)(pair3 + 0) = *(volatile char*)(pair3 + 4);
        len = wcslen((const wchar_t*)key);
        WString_ReplaceRange(&expandedText, v3, 0, key, key + len * 2, pair3);
    }
    dictionaryTag.flags = 0;
    dictionaryTag.value = 0;
    dictionaryTag.allocation = 0;
    key = dartsDialogMessageStrings + kDartsDictionaryTagOpen;
    int v4 = (dictionaryTag.flags >> 31) == 0 ? (dictionaryTag.tag & 0x7F) : dictionaryTag.value;
    *(volatile char*)(pair4 + 0) = *(volatile char*)(pair4 + 4);
    int len = wcslen((const wchar_t*)key);
    WString_ReplaceRange(&dictionaryTag, 0, v4, key, key + len * 2, pair4);
    int v5 = (dictionaryTag.flags >> 31) == 0 ? (dictionaryTag.tag & 0x7F) : dictionaryTag.value;
    *(volatile char*)(pair5 + 0) = *(volatile char*)(pair5 + 4);
    len = wcslen((const wchar_t*)messageKey);
    WString_ReplaceRange(&dictionaryTag, v5, 0, messageKey, messageKey + len * 2, pair5);
    key = dartsDialogMessageStrings + kDartsTagClose;
    int v6 = (dictionaryTag.flags >> 31) == 0 ? (dictionaryTag.tag & 0x7F) : dictionaryTag.value;
    *(volatile char*)(pair6 + 0) = *(volatile char*)(pair6 + 4);
    len = wcslen((const wchar_t*)key);
    WString_ReplaceRange(&dictionaryTag, v6, 0, key, key + len * 2, pair6);
    // Phase 2: expand tags, copy position/size from the layout, set content and
    // start transparent white. Temporary strings die at function exit.
    Text_ExpandTaggedText(((CDartsDialog*)self)->messageResource, &expandedText, (dictionaryTag.flags >> 31) == 0 ? (void*)((char*)&dictionaryTag + 2) : dictionaryTag.allocation, 0);
    Layout_GetElementPosition(((CDartsDialog*)self)->layout, layoutElement, position);
    Text_SetPosition(textDraw, position);
    Layout_GetElementSize(((CDartsDialog*)self)->layout, layoutElement, layoutSize);
    CMsgDraw_SetLayoutSize(textDraw, layoutSize);
    Text_SetContent(textDraw, (expandedText.flags >> 31) == 0 ? (void*)((char*)&expandedText + 2) : expandedText.allocation);
    if (textSlot < 4) CMsgDraw_SetAllGlyphColors(AsCDartsDialog(self)->textDraws[textSlot], -256);
}
#pragma use_lmw_stmw reset

// Create the darts layout, BP entry, number table, and four tagged-text renderers.
void* CDartsDialog_Ctor(void* self, void* targetValue, void* clearType) {
    // Phase 1: initialize loading/fade state and request layout/BP animation.
    // targetValue/clearType carry integer-like words through a void* ABI; this
    // function stores them without taking ownership. BP object is borrowed.
    *(void**)self = __vt__12CDartsDialog;
    AsCDartsDialog(self)->targetValue = targetValue;
    AsCDartsDialog(self)->clearType = clearType;
    (*(int*)&AsCDartsDialog(self)->layout) = 0;
    AsCDartsDialog(self)->bpEntryId = -1;
    AsCDartsDialog(self)->bpObject = 0;
    (*(int*)&AsCDartsDialog(self)->textTable) = 0;
    (*(int*)&AsCDartsDialog(self)->textFactory) = 0;
    (*(int*)&AsCDartsDialog(self)->messageResource) = 0;
    AsCDartsDialog(self)->state = kDartsLoading;
    AsCDartsDialog(self)->fadeState = kDartsFadeComplete;
    AsCDartsDialog(self)->fadeTime = kFloat_0_DartsDialog;
    {
        void* object = __nw__FUl(0x5C);
        if (object != 0) Layout_Init(object, kMenuActivationAction);
        AsCDartsDialog(self)->layout = object;
        Layout_LoadExtended(object, 0, dartsDialogStrings);
    }
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_dartsDialogBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), dartsDialogStrings + kDartsAnimationResource, (void**)&AsCDartsDialog(self)->bpEntryId, 0);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_dartsDialogBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    (*(void**)&AsCDartsDialog(self)->bpObject) = IdMap_Lookup(Sing_805DC5F0(), *(void**)&AsCDartsDialog(self)->bpEntryId);
    if (Sing_805DC830() == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_dartsDialogNumberRegistryDtorRegistration);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    AsCDartsDialog(self)->textTable = CFontNumDraw_CreateInVector(Sing_805DC830());
    // Phase 2: register numeric table, message resource and four text draws.
    // Registry membership and owned draws are released by the matching dtor.
    AsCDartsDialog(self)->textFactory = ((ChatMessageResourceView*)g_chatMsg)->textFactory;
    WFixedFont_CreateMsgEcbResource(AsCDartsDialog(self)->textFactory, &AsCDartsDialog(self)->messageResource, 0, 0);
    {
        char* p = (char*)self;
        void** q = AsCDartsDialog(self)->textDraws;
        unsigned int i = 0;
        int zero = 0;
        while (i < 4) {
            (*(int*)&AsCDartsDialog(p)->textDraw0) = zero;
            WFixedFont_CreateAndRegisterMsgDraw(AsCDartsDialog(self)->textFactory, q, 1);
            ++i;
            q = q + 1;
            p = p + 4;
        }
    }
    AsCDartsDialog(self)->state = kDartsLoading;
    return self;
}

#pragma use_lmw_stmw on
// Unregister and delete darts text, number-table, layout, and BP resources.
void* CDartsDialog_dtor(void* self, int deleting) {
    // Phase 1: unregister then destroy each text draw and message resource.
    // Phase 2: clear/remove numeric table, destroy layout, delete BP entry by ID.
    // Registry lookups are borrowed; never directly free their singleton storage.
    // Phase 3: delete self iff deleting > 0; return its original address.
    struct VObj { 
    virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual void Delete(int) DECOMP_LOCAL_VIRTUAL;
    };
    char* names = s_dartsDialogInputDtorRegistration;
    if (self != 0) {
        *(void**)self = __vt__12CDartsDialog;
        char* p = (char*)self;
        for (unsigned int i = 0; i < 4; ++i) {
            if (Sing_805DC650() == 0) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, (names + 0x24));
                    s_pointerPairListInitialized = 1;
                }
                Sing_805DC650() = s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey(Sing_805DC650(), AsCDartsDialog(p)->textDraw0);
            void* q = AsCDartsDialog(p)->textDraw0;
            if (q != 0) {
                if (q != 0) {
                    struct VObj { 
                    virtual void Delete(int) DECOMP_LOCAL_VIRTUAL;
                    };
                    ((VObj*)q)->Delete(1);
                }
                AsCDartsDialog(p)->textDraw0 = 0;
            }
            p = p + 4;
        }
        void* q = AsCDartsDialog(self)->messageResource;
        if (q != 0) {
            if (q != 0) {
                struct VObj { 
                virtual void Delete(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((VObj*)q)->Delete(1);
            }
            AsCDartsDialog(self)->messageResource = 0;
        }
        UITextTable_ClearEntries(AsCDartsDialog(self)->textTable);
        // Phase 2: remove the owned numeric table through its registry.
        if (AsCDartsDialog(self)->textTable) {
            if (Sing_805DC830() == 0) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (names + 0x18));
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCDartsDialog(self)->textTable);
        }
        if (AsCDartsDialog(self)->bpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (names + 0x0C));
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCDartsDialog(self)->bpEntryId);
            AsCDartsDialog(self)->bpEntryId = -1;
        }
        if (AsCDartsDialog(self)->layout) {
            Layout_Destroy(AsCDartsDialog(self)->layout, 1);
            AsCDartsDialog(self)->layout = 0;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046264C", _ctors_u803371A0_0, SaveLoadMenu_InitFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046264C", _ctors_u803371A0_1, DartsDialog_InitUpdateFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046264C", _ctors_u803371A0_2, DartsDialog_InitDrawFullTurnAngle);
#pragma pop
