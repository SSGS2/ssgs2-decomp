// The darts dialog (CDartsDialog), with its timed fade-in, hold and fade-out sequence, and
// the load-data menu (CLoadDataMenu), which lists four saved player records to load.

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

struct ChatMessageResourceView {
    char padding[0x10];
    void* textFactory;
};
enum DartsDialogState {
    kDartsIdle = 0,
    kDartsLoading = 1,
    kDartsWaitScript = 2,
    kDartsFading = 3
};
enum DartsFadeState {
    kDartsFadeComplete = 0,
    kDartsFadeIn = 1,
    kDartsHold = 2,
    kDartsFadeOut = 3
};
// Text uses packed 0xRRGGBBAA; BP group opacity uses 0..128 for 0..100%.
enum DartsOpacity {
    kDartsWhiteRgb = -256,
    kDartsAlphaMask = 255,
    kDartsGroupOpaque = 128
};
enum SaveLoadUiConstants {
    kSavePlayerSlots = 4,
    kNoSaveSlot = -1,
    kLoadResultNone = 0,
    kLoadResultCancelled = 1,
    kMenuActivationAction = 5
};
#include "classes/CSharedDoc.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadDataMenu.h"
#include "classes/CDartsDialog.h"
#include "classes/CMsgDraw.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpTex.h"
// CDartsDialog::CDartsDialog (0x803371A0): BP entry ID +0x10 and message resource +0x20.
// CLoadDataMenu::CLoadDataMenu (0x80338130): entry IDs +0x20/+0x20EC, message +0x30,
// and the player-record range ends at selectedSlot (+0x20D8).

#define AsCDartsDialog(p) ((CDartsDialog*)(p))

extern "C" {
extern char s_fadeOverlayDtorRegistration_80338838[];

extern float s_saveLoadMenuFullTurnAngle;      // Module angle storage initialized to one full turn.
extern float s_dartsDialogUpdateFullTurnAngle; // Module angle storage initialized to one full turn.
extern float s_dartsDialogDrawFullTurnAngle;   // Module angle storage initialized to one full turn.
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
extern char
    s_dartsDialogInputDtorRegistration[]; // Destructor registration storage for dartsDialogInputDtorRegistration.
extern char
    s_dartsDialogBpCtrlDtorRegistration[]; // Destructor registration storage for dartsDialogBpCtrlDtorRegistration.
extern char s_dartsDialogNumberRegistryDtorRegistration
    []; // Destructor registration storage for dartsDialogNumberRegistryDtorRegistration.

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

void Layout_Update(void*);
void Layout_DrawWithOpacity(void*, float);
void BpObject_Draw(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void UITextTable_Draw(void*);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, void*);
void* PointerVector_InitReserved(void*);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void**, int);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*); // PointerPairList_Dtor.
void* PointerPairList_RemoveByKey(void*, void*);
void UITextTable_ClearEntries(void*);
void* PointerArray_DeleteValue(void*, void*);
void Layout_Destroy(void*, int);
int Layout_PollLoadComplete(void*);
int BpObject_PollLoad(void*);
void Layout_SetElementEnabled(void*, int, int);
void BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetOpacity(void*, int);
void UITextTable_SetInteger(void*, int, void*);
int BpObject_IsCurrentScriptComplete(void*);
void PointerVector_DestroyStorage(void*);
void MessageDrawPairList_Dtor(void*);
void CBpCtrlHandle_Dtor(void*);
void WString_ReplaceRange(void*, int, int, const char*, const char*, char*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Layout_GetElementPosition(void*, int, void*);
void Layout_GetElementSize(void*, int, void*); // Layout_GetElementSize.
void Text_SetPosition(void*, void*);
void Text_SetContent(void*, void*);
int InputManager_IsAnyMenuAdvanceActionActive(void*);
void InputManager_Init(void*);
void InputManager_Dtor(void);
void appFreeUni__FPv(void*);
void* MPBase1_Ctor(void*);
void SaveData_InitPlayerRecord(void*);
void* ResourceTable_FindEntryByName(void*, const char*, unsigned int*);
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
        if (flags >> 31)
            appFreeUni__FPv(allocation);
    }
};

struct TexObj {
    virtual void v0();
    virtual void v1(void*);
};

#pragma opt_common_subs off
// Allocate the save/load layout, initialize four player records, and request UI resources.
CLoadDataMenu::CLoadDataMenu() {
    // Phase 1: initialize base, layout, owned record storage and invalid entry
    // IDs. Phase 2: request layout/BP entries, register text/number renderers.
    // Phase 3: wrap the borrowed archive entry, create BP texture and select
    // the shared document's preview. Returns self; caller supplies storage.
    char* names = s_bpCtrlDtorRecord_SaveLoad;
    MPBase1_Ctor(this);
    *(void**)this = __vt__13CLoadDataMenu;
    void* p = __nw__FUl(0x5C);
    if (p)
        Layout_Init(p, kMenuActivationAction);
    layout = p;
    (*(int*)&bpObject) = 0;
    bpEntryId = -1;
    selectionDelayFrames = -1;
    (*(char*)&finished) = 0;
    textFactory = 0;
    (*(int*)&messageDraw) = 0;
    (*(int*)&textTable) = 0;
    {
        SavePlayerRecord* end;
        SavePlayerRecord* item;
        item = (SavePlayerRecord*)&slotUsed;
        end = (SavePlayerRecord*)&selectedSlot;
        do {
            SaveData_InitPlayerRecord(item);
            ++item;
        } while (item < end);
    }
    selectedSlot = kNoSaveSlot;
    this->result = 0;
    (*(int*)&packTextureResource) = 0;
    (*(int*)&bpTexture) = 0;
    savedScriptIndex = -1;
    secondaryBpEntryId = -1;
    (*(int*)&deleteConfirmationDialog) = 0;
    Memory_Clear(&slotUsed, sizeof(SavePlayerRecord) * kSavePlayerSlots);
    Layout_LoadExtended(layout, 0, saveLoadMenuStrings + kSaveLoadLayout);
    // Phase 2: controller owns BP objects; this menu retains their IDs and
    // borrowed object pointers. Registry membership is removed in the dtor.
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(saveLoadMenuStrings + kSaveLoadAnimation, (void**)&bpEntryId, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    bpObject = IdMap_Lookup(Sing_805DC5F0(), (void*)bpEntryId);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, names + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(saveLoadMenuStrings + kSaveLoadTitleAnimation, (void**)&secondaryBpEntryId, 0);
    void* mgr = ((ChatMessageResourceView*)g_chatMsg)->textFactory;
    (*(void**)&textFactory) = mgr;
    WFixedFont_CreateAndRegisterMsgDraw(mgr, &messageDraw, 1);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (names + 0x30));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    void* list = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    void* mgr620 = Sing_805DC620();
    textTable = list;
    unsigned int size = 0;
    if (!mgr620) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (names + 0x48));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* result = ResourceTable_FindEntryByName(Sing_805DC620(), saveLoadMenuStrings + kSaveLoadTextureArchive,
                                                 (unsigned int*)&size);
    // Phase 3: own wrappers, not the resource table's archive entry itself.
    p = __nw__FUl(0x20);
    if (p) {
        try {
            ((CPackTextureResource*)p)
                ->Init((const char*)(saveLoadMenuStrings + kSaveLoadTextureArchive), (void**)size,
                       (unsigned int)result);
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    packTextureResource = p;
    p = __nw__FUl(0x28);
    if (p)
        ((CBpTex*)p)->__ct();
    bpTexture = p;
    int index = (*(int*)&((CSharedDoc*)g_sharedDoc)->menuTextureIndex);
    void* tex = ((void**)coursePreviewTextureNames)[index];
    ((TexObj*)p)->v1(tex);
}
#pragma opt_common_subs reset

#pragma opt_lifetimes off
// Wait for resources and the opening script, then advance the timed darts dialog.
int CDartsDialog::Update(float delta) {
    // Outer state: loading(1) -> wait script(2) -> fading(3) -> idle(0).
    // The fade enum happens to share 1/2/3; these cases test OUTER state.
    // Every satisfied transition falls through and polls the next phase now.
    // Return 0 while waiting, 1 on completion/default. delta shares the fade
    // clock's units (0.5 transition, 5 hold); no unit conversion occurs here.
    Layout_Update(layout);
    BpObject_TryInvokeCurrentCallback((*(void**)&bpObject), 0);
    int state = this->state;
    switch (state) {
        // 0x80337640: initialization falls through to polling and fading.
    case kDartsLoading: {
        if (!Layout_PollLoadComplete(layout))
            break;
        if (!BpObject_PollLoad((*(void**)&bpObject)))
            break;
        for (unsigned int i = 0; i < 4; ++i)
            Layout_SetElementEnabled(layout, i, 0);
        BpObject_StartScriptByName((*(void**)&bpObject), dartsDialogStrings + kDartsInitialScript, 1);
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
            UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&bpObject), names[i]), 0);
        }
        ((CFontNumDraw*)textTable)
            ->RegisterEntries(&dartsDialogNumberEntryNames, 1, (*(void**)&bpObject),
                              dartsDialogStrings + kDartsNumberFont);
        UITextTable_SetInteger(textTable, 9, targetValue);
        UIGroup_SetOpacity(
            BpObject_GetOrCreateNamedGroup((*(void**)&bpObject), dartsDialogStrings + kDartsTargetNumberGroup), 0);
        dyn_names[0] = (char*)g_dartsPointGroupName;
        dyn_names[1] = (char*)g_dartsDialogOverGroupName;
        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&bpObject), dyn_names[0]), 0);
        UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&bpObject), dyn_names[1]), 0);
        SetTaggedTextAtLayoutElement(0, dartsDialogMessageStrings, dartsDialogMessageStrings + kDartsCenterTag, 0);
        SetTaggedTextAtLayoutElement(1, dartsDialogMessageStrings + kDartsConditionMessage, 0, 1);
        if ((*(int*)&clearType))
            SetTaggedTextAtLayoutElement(3, dartsDialogMessageStrings + kDartsNonzeroClearTypeMessage,
                                         dartsDialogMessageStrings + kDartsRightTag, 3);
        else
            SetTaggedTextAtLayoutElement(3, dartsDialogMessageStrings + kDartsZeroClearTypeMessage,
                                         dartsDialogMessageStrings + kDartsRightTag, 3);
        this->state = kDartsWaitScript;
    }
    case kDartsWaitScript:
        if (!BpObject_IsCurrentScriptComplete((*(void**)&bpObject)))
            break;
        fadeState = kDartsFadeIn;
        fadeTime = kFloat_0_DartsDialog;
        this->state = kDartsFading;
    case kDartsFading:
        if (!UpdateFadeSequence(delta))
            break;
        this->state = kDartsIdle;
    default:
        return 1;
    }
    return 0;
}
#pragma opt_lifetimes reset

// Fade the darts groups/text in, hold until input or timeout, then fade them out.
int CDartsDialog::UpdateFadeSequence(float delta) {
    // Phase diagram: fade in(1, 0.5) -> hold(2, input or 5) -> fade out(3, 0.5)
    // -> complete(0). A completed phase resets time to zero, discarding overshoot,
    // and falls through to process the next phase during THIS call.
    // continue exits the do/while(0), then adds delta ONCE and returns 0.
    // The switch's break reaches return 1, skipping the time increment. This
    // distinction is why replacing continue with break changes fade timing.
    // Text slots 0,1,3 follow alpha; slot 2 is untouched. Window/point/number
    // groups use ceil(alpha/2); the over group is always hidden by this sequence.
    struct Names {
        char* v[7];
    };
    struct Pair {
        const char* v[2];
    };
    CDartsDialog* self = this;
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
                ((CMsgDraw*)(self->textDraws[0]))->SetAllGlyphColors(color);
                ((CMsgDraw*)(self->textDraws[1]))->SetAllGlyphColors(color);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject),
                                                                  dartsDialogStrings + kDartsTargetNumberGroup),
                                   half);
                Pair pair = *(Pair*)&g_dartsPointGroupName;
                pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), half);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                ((CMsgDraw*)(self->textDraws[3]))->SetAllGlyphColors(color);
                continue;
            } else {
                Names names = *(Names*)dartsDialogWindowGroupNames;
                p = names.v;
                for (i = 0; i < 7; ++i, ++p)
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), 128);
                ((CMsgDraw*)(self->textDraws[0]))->SetAllGlyphColors(-1);
                ((CMsgDraw*)(self->textDraws[1]))->SetAllGlyphColors(-1);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject),
                                                                  dartsDialogStrings + kDartsTargetNumberGroup),
                                   128);
                Pair pair = *(Pair*)&g_dartsPointGroupName;
                pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), 128);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                ((CMsgDraw*)(self->textDraws[3]))->SetAllGlyphColors(-1);
            }
            self->fadeState = kDartsHold;
            self->fadeTime = kFloat_0_DartsDialog;
        }
        case kDartsHold:
            if (!((int&)GetInputManagerSlot())) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_dartsDialogInputDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
            }
            if (InputManager_IsAnyMenuAdvanceActionActive((void*)((int&)GetInputManagerSlot())) ||
                !(self->fadeTime < kDartsDialogHoldDuration)) {
                self->fadeState = kDartsFadeOut;
                self->fadeTime = kFloat_0_DartsDialog;
            } else
                continue;
        case kDartsFadeOut: {
            int alpha, half, color;
            char** p;
            unsigned int i;
            if (self->fadeTime < kFloat_0_5_FadeTransition) {
                alpha =
                    (int)(kFloat_255_DartsDialog - kFloat_255_DartsDialog * self->fadeTime / kFloat_0_5_FadeTransition);
                Names names = *(Names*)dartsDialogWindowGroupNames;
                half = ((unsigned char)alpha + 1) / 2;
                p = names.v;
                for (i = 0; i < 7; ++i, ++p)
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), half);
                color = kDartsWhiteRgb | (alpha & kDartsAlphaMask);
                ((CMsgDraw*)(self->textDraws[0]))->SetAllGlyphColors(color);
                ((CMsgDraw*)(self->textDraws[1]))->SetAllGlyphColors(color);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject),
                                                                  dartsDialogStrings + kDartsTargetNumberGroup),
                                   half);
                Pair pair = *(Pair*)&g_dartsPointGroupName;
                pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), half);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                ((CMsgDraw*)(self->textDraws[3]))->SetAllGlyphColors(color);
                continue;
            } else {
                Names names = *(Names*)dartsDialogWindowGroupNames;
                p = names.v;
                for (i = 0; i < 7; ++i, ++p)
                    UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), *p), 0);
                ((CMsgDraw*)(self->textDraws[0]))->SetAllGlyphColors(-256);
                ((CMsgDraw*)(self->textDraws[1]))->SetAllGlyphColors(-256);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject),
                                                                  dartsDialogStrings + kDartsTargetNumberGroup),
                                   0);
                Pair pair = *(Pair*)&g_dartsPointGroupName;
                pair.v[1] = (const char*)g_dartsDialogOverGroupName;
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[0]), 0);
                UIGroup_SetOpacity(BpObject_GetOrCreateNamedGroup((*(void**)&self->bpObject), pair.v[1]), 0);
                ((CMsgDraw*)(self->textDraws[3]))->SetAllGlyphColors(-256);
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
void CLoadDataMenu::PollTextureLoadAndUpdateLayout() {
    void* value = packTextureResource;
    if (value != 0) {
        ((CPackTextureResource*)value)->PollLoad();
    }
    Layout_Update(layout);
}

struct VP0_ {
    virtual void destroy(int);
};
// Release menu textures, text renderers, BP entries, and layout resources.
CLoadDataMenu::~CLoadDataMenu() {
    *(void**)this = __vt__13CLoadDataMenu;
    UITextTable_ClearEntries(textTable);
    if (textTable != 0) {
        if (Sing_805DC830() == 0) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, record30);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), textTable);
    }
    if (Sing_805DC650() == 0) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, record3C);
            s_pointerPairListInitialized = 1;
        }
        Sing_805DC650() = s_messageDrawPairList;
    }
    PointerPairList_RemoveByKey(Sing_805DC650(), messageDraw);
    void* p = messageDraw;
    if (p != 0) {
        if (p != 0) {
            ((VP0_*)p)->destroy(1);
        }
        messageDraw = 0;
    }
    p = layout;
    if (p != 0) {
        Layout_Destroy(p, 1);
        layout = 0;
    }
    if (bpEntryId != -1) {
        // Phase 2: IDs, not bpObject pointers, control BP object lifetime.
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    if (secondaryBpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(secondaryBpEntryId);
        secondaryBpEntryId = -1;
    }
    p = bpTexture;
    if (p != 0) {
        if (p != 0) {
            ((VP0_*)p)->destroy(1);
        }
        bpTexture = 0;
    }
    p = packTextureResource;
    if (p != 0) {
        if (p != 0) {
            ((VP0_*)p)->destroy(1);
        }
        packTextureResource = 0;
    }
    if (Sing_805DC5F8() == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[1] = -1;
            s_bpCtrlHandle[0] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_DartsDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(savedScriptIndex);
}

// Return the load result and write a slot only for a valid non-cancelled selection.
int CLoadDataMenu::GetSelectionResult(int* selectedSlotOut) {
    // Output is unchanged for pending, cancelled or invalid slot; those return
    // 1. Otherwise write 0..3 and return the stored result code unchanged.
    int result = this->result;
    if (result == kLoadResultNone) {
        return 1;
    }
    if (result == kLoadResultCancelled) {
        return 1;
    }
    int selectedSlot = this->selectedSlot;
    if (selectedSlot < 0 || selectedSlot >= kSavePlayerSlots) {
        return 1;
    }
    *selectedSlotOut = selectedSlot;
    return this->result;
}

// Draw the loaded darts layout, BP animation, and numeric text table.
void CDartsDialog::Draw() {
    switch (state) {
    case kDartsLoading:
        return;
    default:
        Layout_DrawWithOpacity(layout, kFloat_1_DartsDialogOpacity);
        BpObject_Draw((*(void**)&bpObject));
        UITextTable_Draw(textTable);
        return;
    }
}

#pragma use_lmw_stmw on
// Expand a message key with an optional alignment tag and place it at a layout element.
void CDartsDialog::SetTaggedTextAtLayoutElement(int textSlot, const char* messageKey, const char* alignmentTag,
                                                int layoutElement) {
    // Inputs point to UTF-16 text despite their char* ABI. Caller must supply
    // 0 <= textSlot < 4 (only the upper bound is checked), a valid layout ID,
    // and terminated keys/tags. ParseResult owns temporary heap strings.
    // Phase 1: build optional alignment markup and the dictionary lookup tag.
    if (textSlot >= 4)
        return;
    ParseResult expandedText;
    ParseResult dictionaryTag;
    void* textDraw = textDraws[textSlot];
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
        v1 = (expandedText.flags >> 31) == 0 ? (expandedText.tag & 0x7F) : expandedText.value;
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
    Text_ExpandTaggedText(
        messageResource, &expandedText,
        (dictionaryTag.flags >> 31) == 0 ? (void*)((char*)&dictionaryTag + 2) : dictionaryTag.allocation, 0);
    Layout_GetElementPosition(layout, layoutElement, position);
    Text_SetPosition(textDraw, position);
    Layout_GetElementSize(layout, layoutElement, layoutSize);
    ((CMsgDraw*)textDraw)->SetLayoutSize((float*)layoutSize);
    Text_SetContent(textDraw,
                    (expandedText.flags >> 31) == 0 ? (void*)((char*)&expandedText + 2) : expandedText.allocation);
    if (textSlot < 4)
        ((CMsgDraw*)(textDraws[textSlot]))->SetAllGlyphColors(-256);
}
#pragma use_lmw_stmw reset

// Create the darts layout, BP entry, number table, and four tagged-text renderers.
CDartsDialog::CDartsDialog(void* targetValue, void* clearType) {
    // Phase 1: initialize loading/fade state and request layout/BP animation.
    // targetValue/clearType carry integer-like words through a void* ABI; this
    // function stores them without taking ownership. BP object is borrowed.
    *(void**)this = __vt__12CDartsDialog;
    this->targetValue = targetValue;
    this->clearType = clearType;
    (*(int*)&layout) = 0;
    bpEntryId = -1;
    bpObject = 0;
    (*(int*)&textTable) = 0;
    (*(int*)&textFactory) = 0;
    (*(int*)&messageResource) = 0;
    state = kDartsLoading;
    fadeState = kDartsFadeComplete;
    fadeTime = kFloat_0_DartsDialog;
    {
        void* object = __nw__FUl(0x5C);
        if (object != 0)
            Layout_Init(object, kMenuActivationAction);
        layout = object;
        Layout_LoadExtended(object, 0, dartsDialogStrings);
    }
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_dartsDialogBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(dartsDialogStrings + kDartsAnimationResource, (void**)&bpEntryId, 0);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_dartsDialogBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    (*(void**)&bpObject) = IdMap_Lookup(Sing_805DC5F0(), *(void**)&bpEntryId);
    if (Sing_805DC830() == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     s_dartsDialogNumberRegistryDtorRegistration);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    textTable = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    // Phase 2: register numeric table, message resource and four text draws.
    // Registry membership and owned draws are released by the matching dtor.
    textFactory = ((ChatMessageResourceView*)g_chatMsg)->textFactory;
    WFixedFont_CreateMsgEcbResource(textFactory, &messageResource, 0, 0);
    {
        char* p = (char*)this;
        void** q = textDraws;
        unsigned int i = 0;
        int zero = 0;
        while (i < 4) {
            (*(int*)&AsCDartsDialog(p)->textDraw0) = zero;
            WFixedFont_CreateAndRegisterMsgDraw(textFactory, q, 1);
            ++i;
            q = q + 1;
            p = p + 4;
        }
    }
    state = kDartsLoading;
}

#pragma use_lmw_stmw on
// Unregister and delete darts text, number-table, layout, and BP resources.
// The destructor's body. It stays a named member: as a real destructor it compiles with its loads in a different order.
void* CDartsDialog::Destroy(int deleting) {
    // Phase 1: unregister then destroy each text draw and message resource.
    // Phase 2: clear/remove numeric table, destroy layout, delete BP entry by ID.
    // Registry lookups are borrowed; never directly free their singleton storage.
    // Phase 3: delete this iff deleting > 0; return its original address.
    struct VObj {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void b() DECOMP_LOCAL_VIRTUAL;
        virtual void Delete(int) DECOMP_LOCAL_VIRTUAL;
    };
    char* names = s_dartsDialogInputDtorRegistration;
    if (this != 0) {
        *(void**)this = __vt__12CDartsDialog;
        char* p = (char*)this;
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
        void* q = AsCDartsDialog(this)->messageResource;
        if (q != 0) {
            if (q != 0) {
                struct VObj {
                    virtual void Delete(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((VObj*)q)->Delete(1);
            }
            AsCDartsDialog(this)->messageResource = 0;
        }
        UITextTable_ClearEntries(AsCDartsDialog(this)->textTable);
        // Phase 2: remove the owned numeric table through its registry.
        if (AsCDartsDialog(this)->textTable) {
            if (Sing_805DC830() == 0) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                             (names + 0x18));
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCDartsDialog(this)->textTable);
        }
        if (AsCDartsDialog(this)->bpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (names + 0x0C));
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(AsCDartsDialog(this)->bpEntryId);
            AsCDartsDialog(this)->bpEntryId = -1;
        }
        if (AsCDartsDialog(this)->layout) {
            Layout_Destroy(AsCDartsDialog(this)->layout, 1);
            AsCDartsDialog(this)->layout = 0;
        }
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
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
