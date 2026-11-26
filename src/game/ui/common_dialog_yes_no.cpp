// The modal Yes/No/OK dialog: it loads a six-element button layout and two Bp
// scripts, shows a tagged UTF-16 message, and fades the whole window in and out.
// One process call advances the fade and the input delay, then turns the first
// pressed button into an accepted or cancelled result.
//
// The layout, scripts and font are shared game services; this class owns the
// layout object, the message renderer and the tag expander it creates, plus the
// text it stores, and gives the two Bp entries back when it is destroyed.
#include <decomp/local_class.h>
#include "classes/CPangyaCommonDialog.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CMsgDraw.h"
#include "classes/WFixedFont.h"
#include "game/singletons.h"
#include "game/wstring.h"
#include "classes/CMouseCursor.h"
#include "classes/CBpCtrl.h"

// Offsets into commonDialogStrings.
enum CommonDialogStringsOffset {
    kCommonDialogStrings_common_dialogue_spd = 0x14,    // common_dialogue.spd
    kCommonDialogStrings_black_sprite_anm_b2d = 0x28,   // black_sprite_anm.b2d
    kCommonDialogStrings_black_sprite = 0x3D,           // black_sprite
    kCommonDialogStrings_FontRec = 0x4A,                // FontRec
    kCommonDialogStrings_ui_button_ok_click = 0x52,     // ui_button_ok_click
    kCommonDialogStrings_ui_button_cancel_click = 0x65, // ui_button_cancel_click
    kCommonDialogStrings_R_dlwin_l_over = 0x7C,         // R_dlwin_l_over
    kCommonDialogStrings_window = 0x8B,                 // window
};

// The layout elements: 0 is OK/Yes, 1 is Cancel/No, 2 is the third action, and
// each button has a paired hit-state element three slots later.
enum CommonDialogButton {
    kDialogOk = 0,
    kDialogCancel = 1,
    kDialogThirdAcceptAction = 2
};
enum CommonDialogResult {
    kDialogPending = 0,
    kDialogAccepted = 1,
    kDialogCancelled = 2
};
// Four process calls are skipped after an activation; the count is in calls,
// not in frames or milliseconds.
enum CommonDialogTiming {
    kDialogInputDelayUpdates = 4,
    kDialogButtonCount = 3,
    kDialogLayoutElementCount = 6
};
// Kind 3 is the acknowledge dialog (Yes/No disabled) and kind 4 the buttonless
// one (all three disabled); these two share the userdata_dialogue script.
enum CommonDialogKind {
    kDialogAcknowledgeKind = 3,
    kDialogButtonlessKind = 4
};
enum {
    kDialogPairedElementOffset = 3,
    kDialogAuxiliaryGroupId = 0x3EE
};
enum CommonDialogOpacity {
    kDialogOpaque = 255,
    kDialogScriptOpaque = 0x80,
    kDialogColorBias = 0x100
};

// The UTF-16 text starts two bytes into the WString header.
enum {
    kInlineTextOffset = (int)&((CPangyaCommonDialog*)0)->textFlags + 2
};

extern "C" {

extern void* g_mouseCursor;
extern void* g_chatMsg;
extern void* g_soundManager;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_8050A210[];
extern char commonDialogStrings[];
extern char* commonDialogKindScriptNames[];
extern void* commonDialogButtonGroupNames[];
extern char s_commonDialogLayoutElements[];
extern void* s_commonDialogueResourcesReady;
extern float kFloat_neg1_CommonDialogCursor;
extern float kFloat_0_5_CommonDialogSound;
extern float kFloat_200_CommonDialogSound;
extern float kFloat_1_CommonDialog;
extern char __vt__19CPangyaCommonDialog[];

// The Bp script handle singleton (scriptIndex at +4).
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleDtorRegistration_CommonDialog[];
// Input manager singleton.
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_CommonDialog[];

void Layout_Init(void* self, int mode);
void Layout_InitStandard(void* self, int player, const char* elements, int count, const char* script);
void Layout_Update(void* self);
int Layout_PollLoadComplete(void* self);
void Layout_Destroy(void* self, int deleting);
int Layout_CollectActivated(void* self, void* out, int player);
void Layout_SetElementPosition(void* self, int index, void* point);
void Layout_GetElementPosition(void* self, int index, void* point);
void Layout_GetElementSize(void* self, int index, void* point);
void Layout_WriteElementRect(void* self, int index, void* rect);
void Layout_SetElementColor(void* self, int index, int value);
void Layout_SetElementSecondaryState(void* self, int index, int value);
void Layout_SetElementPlayerFlagsAndState(void* self, int index, int player);
void Layout_SetElementEnabled(void* self, int index, int player);
void Layout_DrawWithOpacity(void* self, float opacity);

void CBpCtrl_dtor(void* self);
void BpCtrl_SetOpacity(void* entry, int value);

void CBpCtrlHandle_Dtor(void* self);

void* IdMap_Lookup(void* self, int index);
void* BpObject_GetOrCreateNamedGroup(void* self, const char* name);
void* UIGroup_GetOrCreateById(void* self, int id);
void UIGroup_GetPosition(void* self, void* out);
void UIGroup_GetSourceSize(void* self, void* out);
void UIGroup_SetEnabled(void* self, int enabled);
int BpObject_PollLoad(void* self);
void BpObject_StartScriptByName(void* self, const char* name, int mode);
void BpObject_TryInvokeCurrentCallback(void* self, int value);
void BpObject_Draw(void* self);
void BpSlot_SetUserData(void* entry, int x, int y);
void B2dGetInfoPtr(void* entry, int* x, int* y);

void Text_SetPosition(void* self, void* position);
void Text_SetContent(void* self, void* content);
void Text_DrawGlyphs(void* self);
void Text_ExpandTaggedText(void* self, void* out, void* text, int flag);
void WFixedFont_CreateMsgEcbResource(void* self, void* out, int a, int b);

void InputManager_Init(void* self);
void InputManager_Dtor(void* self);
int InputManager_GetSelectedPlayer(void* self);
void Sound_PlayByName(void* self, const char* name, int a, int b, int c, int d, float volume, float distance,
                      float pitch);
void WString_ReplaceRange(void* self, void* index, int count, int value, int a, int b);

void* __register_global_object(void* object, void* dtor, void* registration);
void* __nw__FUl(unsigned int size);
void __dl__FPv(void* self);
void appFreeUni__FPv(void* self);

} // extern "C"

// Fields of the message manager singleton that this file reads.
struct DialogMessageManager {
    char pad_00[0x10];
    WFixedFont* font; // 0x10
};
// An object this file only ever deletes through its first vtable slot.
struct DeletableObject {
    virtual void Destroy(int deleting) DECOMP_LOCAL_VIRTUAL;
};
// The dialog's own vtable. Slot 4 receives the low byte of the new opacity.
struct CommonDialogVtable {
    virtual void vfunc_0() DECOMP_LOCAL_VIRTUAL;
    virtual void vfunc_1() DECOMP_LOCAL_VIRTUAL;
    virtual void vfunc_2() DECOMP_LOCAL_VIRTUAL;
    virtual void vfunc_3() DECOMP_LOCAL_VIRTUAL;
    virtual void SetAlpha(int alpha) DECOMP_LOCAL_VIRTUAL; // name inferred from the value it takes
};
// The ID list Layout_CollectActivated fills in. Only the fields this file reads
// are named; the destructor frees the collection.
struct LayoutIdList {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // The unsigned subtraction keeps the original count-clear code.
    ~LayoutIdList() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};
// The input layout object, allocated here and constructed by Layout_Init.
struct LayoutObject {
    char pad_00[0x5C];
    LayoutObject(int mode) {
        Layout_Init(this, mode);
    }
};
// A message renderer, constructed against the font's message resource.
struct MsgDrawObject {
    char pad_00[0x4BC];
    MsgDrawObject(void* resource) {
        ((CMsgDraw*)this)->__ct(resource);
    }
};

// Float constants, still linked from the original data.
#define kCursorSnapDuration kFloat_neg1_CommonDialogCursor // -1 makes the cursor snap
#define kClickSoundVolume kFloat_0_5_CommonDialogSound
#define kClickSoundDistance kFloat_200_CommonDialogSound
#define kOne kFloat_1_CommonDialog

// Construct the dialog around the button layout for a kind, then load and fade it in.
// Construction blocks until the scripts and font are ready.
CPangyaCommonDialog::CPangyaCommonDialog(void* arg, int fadeDuration) {
    kind = (int)arg;
    vtbl = __vt__19CPangyaCommonDialog;
    result = 0;
    inputDelay = 0;
    layout = 0;
    backgroundEntryId = -1;
    dialogEntryId = -1;
    message = 0;
    textFlags = 0;
    textLength = 0;
    textData = 0;
    opacity = kDialogOpaque;
    opacityStep = kDialogOpaque;
    this->fadeDuration = fadeDuration;
    savedScriptIndex = -1;
    // Phase 1: allocate the owned input layout, then resolve its input manager.
    void* newLayout = new LayoutObject(5);
    volatile void* inputManagerSnapshot = GetInputManagerSlot();
    // Both arms store newLayout; the self-comparison is how the original compiles.
    layout = (inputManagerSnapshot == inputManagerSnapshot) ? newLayout : newLayout;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_CommonDialog);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    Layout_InitStandard(layout, InputManager_GetSelectedPlayer(GetInputManagerSlot()), s_commonDialogLayoutElements,
                        kDialogLayoutElementCount, commonDialogStrings + kCommonDialogStrings_common_dialogue_spd);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Phase 2: create the background and dialog script entries, then build the text.
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(commonDialogStrings + kCommonDialogStrings_black_sprite_anm_b2d, &backgroundEntryId, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(commonDialogStrings, &dialogEntryId, 0);
    FinishLoading();
    StartFade(0, 0);
    StartFade(1, this->fadeDuration);
}

// Release the owned layout, renderer, tag expander and script entries, then put
// the global script index back. The text is freed as the last step.
CPangyaCommonDialog::~CPangyaCommonDialog() {
    vtbl = __vt__19CPangyaCommonDialog;
    if (layout) {
        Layout_Destroy(layout, 1);
        layout = 0;
    }
    if (message) {
        if (message)
            ((DeletableObject*)message)->Destroy(1);
        message = 0;
    }
    if (tagExpander) {
        if (tagExpander)
            ((DeletableObject*)tagExpander)->Destroy(1);
        tagExpander = 0;
    }
    BpCtrl_SetOpacity((*(void**)&dialogEntryId), kDialogScriptOpaque);
    BpCtrl_SetOpacity((*(void**)&backgroundEntryId), kDialogScriptOpaque);
    if (dialogEntryId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(dialogEntryId);
        dialogEntryId = -1;
    }
    if (backgroundEntryId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(backgroundEntryId);
        backgroundEntryId = -1;
    }
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_CommonDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(savedScriptIndex);
    // The stored text is inline unless its high flag bit marks heap storage.
    if (&textFlags && (textFlags >> 31) != 0)
        appFreeUni__FPv(textData);
}

// Wait for the layout, background and dialog scripts, then create the owned text
// objects; the last phase saves the global script index before releasing it.
void CPangyaCommonDialog::FinishLoading() {
    if (!s_commonDialogueResourcesReady)
        return;
    // Phase 1: block until the layout, background and dialog scripts are ready.
    while (!Layout_PollLoadComplete(layout))
        Layout_Update(layout);
    void* node;
    void* iter;
    for (;;) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        iter = IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId);
        if (BpObject_PollLoad(iter))
            break;
    }
    for (;;) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        iter = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
        if (BpObject_PollLoad(iter))
            break;
    }
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    while (!BpObject_PollLoad(node)) {
    }
    // Phase 2: choose the kind script and align the buttons, then build the text.
    BpObject_StartScriptByName(node, commonDialogKindScriptNames[kind], 1);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId);
    BpObject_StartScriptByName(node, commonDialogStrings + kCommonDialogStrings_black_sprite, 1);
    ConfigureButtonLayout();
    void* obj;
    void* manager = g_chatMsg;
    void* mgrObj = ((DialogMessageManager*)manager)->font;
    WFixedFont_CreateMsgEcbResource(mgrObj, &tagExpander, 0, 0);
    obj = new MsgDrawObject(((WFixedFont*)mgrObj)->messageResource);
    message = obj;
    // Phase 3: erase the current UTF-16 span and attach the empty dialog text.
    // The text runs from the inline buffer (or textData) for textLength units;
    // the repeated base reloads reconstruct the original iterator subtraction.
    int textStart, textUnits;
    if (!(textFlags >> 31)) {
        textStart = (int)(((char*)this + kInlineTextOffset));
        textUnits = (*(unsigned char*)&textFlags) & 0x7f;
    } else {
        textStart = (int)textData;
        textUnits = textLength;
    }
    int end = textStart + textUnits * 2;
    int a2 = (int)((!(textFlags >> 31)) ? (void*)(((char*)this + kInlineTextOffset)) : textData);
    int spanBytes = end - a2;
    int count = (((int)((unsigned int)spanBytes >> 31)) + spanBytes) >> 1;
    int diff2 = a2 - (int)((!(textFlags >> 31)) ? (void*)(((char*)this + kInlineTextOffset)) : textData);
    // `bytes` doubles as scratch for the replace call and as position/rectangle
    // storage below; `fill` is only the allocator token.
    struct {
        int fill;
        volatile unsigned char pad;
        char unused[3];
        int pos[2];
        float p[4];
    } bytes;
    *(unsigned char*)&bytes.fill = bytes.pad;
    WString_ReplaceRange((void*)&textFlags, (void*)(diff2 / 2), count, 0, 0, (int)&bytes.fill);
    ((CMsgDraw*)message)->ResetTextState();
    ((CMsgDraw*)message)->ClearFormattingAndRevealAll();
    Text_SetContent(message, (!(textFlags >> 31)) ? (void*)(((char*)this + kInlineTextOffset)) : textData);
    // Phase 4: place the text in the FontRec rectangle relative to the dialog entry.
    bytes.pos[0] = 0;
    bytes.pos[1] = 0;
    B2dGetInfoPtr((*(void**)&dialogEntryId), &bytes.pos[0], &bytes.pos[1]);
    *(int*)&bytes.p[0] = 0;
    *(int*)&bytes.p[1] = 0;
    *(int*)&bytes.p[2] = 0;
    *(int*)&bytes.p[3] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    void* pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_FontRec);
    UIGroup_GetPosition(pane, &bytes.p[0]);
    bytes.p[0] += bytes.pos[0];
    bytes.p[1] += bytes.pos[1];
    UIGroup_GetSourceSize(pane, &bytes.p[2]);
    Text_SetPosition(message, &bytes.p[0]);
    ((CMsgDraw*)message)->SetLayoutSize((float*)&bytes.p[2]);
    ((CMsgDraw*)message)->ResetTextState();
    ((CMsgDraw*)message)->ClearFormattingAndRevealAll();
    Text_SetContent(message, (!(textFlags >> 31)) ? (void*)(((char*)this + kInlineTextOffset)) : textData);
    // Phase 5: clear the paired states, save the global script index and release
    // its entry. The dialog keeps only the index value, not the handle.
    for (int i = 0; i < 6; i++)
        Layout_SetElementSecondaryState(layout, i, 0);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_CommonDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    (*(void**)&savedScriptIndex) = (*(void**)&((CBpCtrlHandle*)Sing_805DC5F8())->scriptIndex);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_CommonDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
}

// Advance the fade and the input delay, then map the first activated button to a
// result. The return value is a readiness flag, so a result shows up one call later.
int CPangyaCommonDialog::UpdateFadeAndInput() {
    if (s_commonDialogueResourcesReady == 0)
        return 0;
    Layout_Update(layout);
    int timer = inputDelay;
    inputDelay = timer - 1;
    if (timer > 0)
        return 0;
    if (fading) {
        int nextOpacity = opacity + opacityStep;
        opacity = nextOpacity;
        if (nextOpacity < 0) {
            opacity = 0;
            fading = 0;
        }
        if (opacity > kDialogOpaque) {
            opacity = kDialogOpaque;
            fading = 0;
        }
        ((CommonDialogVtable*)this)->SetAlpha(opacity & 0xff);
        return 0;
    }
    if (result)
        return 1;
    LayoutIdList activatedButtons = {0, 0, 0};
    if (Layout_CollectActivated(layout, &activatedButtons, 0)) {
        int button = *(int*)activatedButtons.items;
        if (button == kDialogThirdAcceptAction || button == kDialogOk) {
            Sound_PlayByName(g_soundManager, commonDialogStrings + kCommonDialogStrings_ui_button_ok_click, 0, 0, 0, 0,
                             kClickSoundVolume, kClickSoundDistance, kOne);
            result = kDialogAccepted;
        } else if (button == kDialogCancel) {
            Sound_PlayByName(g_soundManager, commonDialogStrings + kCommonDialogStrings_ui_button_cancel_click, 0, 0, 0,
                             0, kClickSoundVolume, kClickSoundDistance, kOne);
            result = kDialogCancelled;
        }
        Layout_SetElementPlayerFlagsAndState(layout, *(int*)activatedButtons.items, 0);
        Layout_SetElementEnabled(layout, *(int*)activatedButtons.items + kDialogPairedElementOffset, 1);
        inputDelay = kDialogInputDelayUpdates;
    }
    return 0;
}

// Draw the background, the window, then the revealed text and the button layout.
void CPangyaCommonDialog::Draw() {
    void* bpObject;
    if (s_commonDialogueResourcesReady == 0)
        return;
    {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        bpObject = IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId);
        BpObject_TryInvokeCurrentCallback(bpObject, 0);
        BpObject_Draw(bpObject);
    }
    {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        bpObject = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
        BpObject_TryInvokeCurrentCallback(bpObject, 0);
        BpObject_Draw(bpObject);
    }
    ((CMsgDraw*)message)->UpdateReveal();
    void* obj = message;
    ((CMsgDraw*)obj)->drawSuppressed = 0;
    Text_DrawGlyphs(obj);
    Layout_DrawWithOpacity(layout, kOne);
}

// Move the window to (x, y), then re-align the button hit rectangles and text.
void CPangyaCommonDialog::SetPosition(int x, int y) {
    struct {
        float p[4];
        float pos[2];
        float anchorPosition[2];
    } buffers;
    *(int*)&buffers.anchorPosition[0] = 0;
    *(int*)&buffers.anchorPosition[1] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* node = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    void* pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_R_dlwin_l_over);
    UIGroup_GetPosition(pane, buffers.anchorPosition);
    *(int*)&buffers.pos[0] = 0;
    *(int*)&buffers.pos[1] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_window);
    UIGroup_GetPosition(pane, buffers.pos);
    BpSlot_SetUserData((void*)dialogEntryId, (int)((float)x - buffers.anchorPosition[0] - buffers.pos[0]),
                       (int)((float)y - buffers.anchorPosition[1] - buffers.pos[1]));
    ConfigureButtonLayout();
    int coords[2] = {0, 0};
    B2dGetInfoPtr((void*)dialogEntryId, &coords[0], &coords[1]);
    *(int*)&buffers.p[0] = 0;
    *(int*)&buffers.p[1] = 0;
    *(int*)&buffers.p[2] = 0;
    *(int*)&buffers.p[3] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_FontRec);
    UIGroup_GetPosition(pane, &buffers.p[0]);
    buffers.p[0] = buffers.p[0] + (float)coords[0];
    buffers.p[1] = buffers.p[1] + (float)coords[1];
    UIGroup_GetSourceSize(pane, &buffers.p[2]);
    Text_SetPosition(message, &buffers.p[0]);
    ((CMsgDraw*)message)->SetLayoutSize((float*)&buffers.p[2]);
    ((CMsgDraw*)message)->ResetTextState();
    ((CMsgDraw*)message)->ClearFormattingAndRevealAll();
    Text_SetContent(message, !(textFlags >> 31) ? ((char*)this + kInlineTextOffset) : textData);
}

// Move the mouse cursor onto a button; the duration -1 makes it snap.
void CPangyaCommonDialog::MoveCursorToButton(int index) {
    float buf[2];
    *(int*)buf = 0;
    *(int*)(buf + 1) = 0;
    Layout_GetElementPosition(layout, index, buf);
    ((CMouseCursor*)g_mouseCursor)->MoveTo(buf[0], buf[1], kCursorSnapDuration);
}

// Apply the opacity to the layout, both script entries and the message text, then
// rebind and fully reveal the stored text.
void CPangyaCommonDialog::ApplyOpacity(int value) {
    int i;
    int tint = value - kDialogColorBias;
    for (i = 0; i < kDialogLayoutElementCount; ++i)
        Layout_SetElementColor(layout, i, tint);
    int scaled = ((value & 0xff) << 7) / kDialogOpaque;
    BpCtrl_SetOpacity((*(void**)&backgroundEntryId), scaled);
    BpCtrl_SetOpacity((*(void**)&dialogEntryId), scaled);
    ((CMsgDraw*)message)->colorMultiplier = tint;
    ((CMsgDraw*)message)->ResetTextState();
    ((CMsgDraw*)message)->ClearFormattingAndRevealAll();
    void* obj = message;
    void* text = !(textFlags >> 31) ? ((char*)this + kInlineTextOffset) : textData;
    Text_SetContent(obj, text);
}

// Align the three button/hit-state pairs with the dialog entry, then enable the
// buttons the kind leaves available.
void CPangyaCommonDialog::ConfigureButtonLayout() {
    int left = 0, top = 0;
    B2dGetInfoPtr((*(void**)&dialogEntryId), &left, &top);
    int i;
    for (i = 0; i < kDialogButtonCount; ++i) {
        float pos[2];
        *(int*)&pos[0] = 0;
        *(int*)&pos[1] = 0;
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* node = IdMap_Lookup(Sing_805DC5F0(), (int)dialogEntryId);
        void* pane = BpObject_GetOrCreateNamedGroup(node, (const char*)commonDialogButtonGroupNames[i]);
        UIGroup_GetPosition(pane, pos);
        UIGroup_SetEnabled(pane, 0);
        pos[0] = pos[0] + (float)left;
        pos[1] = pos[1] + (float)top;
        int point[2];
        point[0] = *(int*)&pos[0];
        point[1] = *(int*)&pos[1];
        Layout_SetElementPosition(layout, i, point);
        Layout_GetElementSize(layout, i, point);
        int rect[4];
        rect[0] = (int)pos[0];
        rect[1] = (int)pos[1];
        rect[2] = (int)(pos[0] + *(float*)&point[0]);
        rect[3] = (int)(pos[1] + *(float*)&point[1]);
        Layout_WriteElementRect(layout, i, rect);
        int point2[2];
        point2[0] = *(int*)&pos[0];
        point2[1] = *(int*)&pos[1];
        Layout_SetElementPosition(layout, i + kDialogPairedElementOffset, point2);
        Layout_GetElementSize(layout, i + kDialogPairedElementOffset, point2);
        int rect2[4];
        rect2[0] = (int)pos[0];
        rect2[1] = (int)pos[1];
        rect2[2] = (int)(pos[0] + *(float*)&point2[0]);
        rect2[3] = (int)(pos[1] + *(float*)&point2[1]);
        Layout_WriteElementRect(layout, i + kDialogPairedElementOffset, rect2);
    }
    // Clear the states the kind disables, then the three paired hit elements.
    int kind = this->kind;
    if (kind == kDialogAcknowledgeKind) {
        Layout_SetElementPlayerFlagsAndState(layout, 0, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 1, 0);
    } else if (kind == kDialogButtonlessKind) {
        Layout_SetElementPlayerFlagsAndState(layout, 0, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 1, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 2, 0);
    } else {
        Layout_SetElementPlayerFlagsAndState(layout, 2, 0);
    }
    int element;
    for (element = 3; element < 6; ++element) {
        Layout_SetElementPlayerFlagsAndState(layout, element, 0);
    }
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    UIGroup_SetEnabled(
        UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), (int)dialogEntryId), kDialogAuxiliaryGroupId), 0);
}

#pragma opt_common_subs off
// Clear the stored text, expand the tags of the replacement into it, then rebuild
// the message formatting against the new text.
void CPangyaCommonDialog::SetText(void* taggedText) {
    // Phase 1: erase [0, old UTF-16 length). Inline text starts at textFlags + 2;
    // heap text uses textData and textLength. The repeated base reloads reproduce
    // the original iterator subtraction: half2 is the start index (zero), half1
    // the erased code-unit count. Integer addresses below are byte addresses.
    unsigned int strFlag = textFlags;
    int heapAllocated = (int)(strFlag >> 31);
    int lengthOrTextAddress; // Length first, then the text address for the second span.
    int base;
    if (!heapAllocated) {
        lengthOrTextAddress = (*(unsigned char*)&textFlags) & 0x7f;
        base = (int)(((char*)this + kInlineTextOffset));
    } else {
        base = (*(int*)&textData);
        lengthOrTextAddress = textLength;
    }
    int end = base + lengthOrTextAddress * 2;
    int inlineText = !heapAllocated;
    base = inlineText ? (int)(((char*)this + kInlineTextOffset)) : (*(int*)&textData);
    int span1 = end - base;
    int half1 = span1 / 2;
    lengthOrTextAddress = inlineText ? (int)(((char*)this + kInlineTextOffset)) : (*(int*)&textData);
    int span2 = base - lengthOrTextAddress;
    int half2 = span2 / 2;
    // Allocator-token storage, not a text character; its incoming byte is left
    // unspecified here, so it is not a fill value.
    volatile unsigned char pad;
    int fill;
    *(unsigned char*)&fill = pad;
    WString_ReplaceRange(&textFlags, (void*)half2, half1, 0, 0, (int)&fill);
    // Phase 2: expand into an owning temporary, then expand again into stored text.
    if (taggedText) {
        WString temp;
        Text_ExpandTaggedText(tagExpander, &temp, taggedText, 0);
        void* src = !(temp.flags >> 31) ? (char*)&temp + 2 : (char*)temp.data;
        Text_ExpandTaggedText(tagExpander, &textFlags, src, 0);
    }
    // Phase 3: rebuild the message formatting against the dialog-owned text.
    ((CMsgDraw*)message)->ResetTextState();
    ((CMsgDraw*)message)->ClearFormattingAndRevealAll();
    void* text = !(textFlags >> 31) ? ((char*)this + kInlineTextOffset) : textData;
    Text_SetContent(message, text);
}
#pragma opt_common_subs reset

// Start an immediate opacity change or a stepped fade over process calls. A zero
// duration applies the endpoint at once; otherwise the current opacity is kept.
void CPangyaCommonDialog::StartFade(int fadeIn, int durationFrames) {
    if (fadeIn != 0) {
        if (durationFrames == 0) {
            int notFading = 0;
            int opacityOrStep = kDialogOpaque;
            fading = notFading;
            opacity = opacityOrStep;
        } else {
            int opacityOrStep = kDialogOpaque / durationFrames;
            fading = 1;
            opacityStep = opacityOrStep;
        }
    } else {
        if (durationFrames == 0) {
            int opacityOrStep = 0;
            fading = opacityOrStep;
            opacity = opacityOrStep;
        } else {
            int opacityOrStep = -kDialogOpaque / durationFrames;
            fading = 1;
            opacityStep = opacityOrStep;
        }
    }
    int currentOpacity = opacity;
    ((CommonDialogVtable*)this)->SetAlpha(currentOpacity & 0xff);
}

// Enable or disable the window group if the loaded dialog has one.
void CPangyaCommonDialog::SetWindowEnabled(int enabled) {
    if (*(volatile void**)&Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* h = IdMap_Lookup(Sing_805DC5F0(), dialogEntryId);
    void* style = BpObject_GetOrCreateNamedGroup(h, commonDialogStrings + kCommonDialogStrings_window);
    if (style != 0)
        UIGroup_SetEnabled(style, enabled);
}
