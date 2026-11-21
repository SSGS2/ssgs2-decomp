#include <decomp/local_class.h>
// Yes/No/OK common dialog: input layout, Bp scripts, tagged message and fades.
// Modding lookup (controls / YES-NO dialog / frame rate):
// - CommonDialogButton maps activations 0/1/2; the third R_btn_dl action also
//   accepts the acknowledgement action used by kind 3. Button names and
//   kind scripts live in u80279C64_data.cpp and s_commonDialogLayoutElements.
// - CPangyaCommonDialog_UpdateFadeAndInput consumes ONLY the first activated ID. It stores
//   result 1=accepted, 2=cancelled, but returns completion on a LATER call. The
//   return value is a readiness flag, not the chosen answer. Preserve that delay.
// - kDialogInputDelayUpdates=4 suppresses four process calls after an activation;
//   StartFade's duration argument is also process calls, not milliseconds/Hz.
//   Integer alpha steps truncate; see StartFade for endpoint and stall behavior.
// - CommonDialog_SetPosition moves Bp, text and all six hit rectangles together;
//   keep those spaces aligned for widescreen. Coordinates are layout pixels,
//   top-left origin, x right/y down; integer hit bounds truncate float positions.
// Construction owns layout, message, tagExpander, text storage and two Bp entry
// IDs. FinishLoading blocks on polls, borrows font services and saves/releases
// the global script index; dtor restores it after deleting owned entries.
// Shared services/singleton buffers are borrowed and remain registered globally.
// Required self is a fully initialized dialog except during construction;
// rendering/input must run serially with destruction. Resource-ready==0 makes
// update/draw/FinishLoading return without work; it does not complete loading.
// CPangyaCommonDialogFields.h supplies the shared 0x44-byte ABI layout. Text is
// NUL-terminated UTF-16: flag high bit selects heap storage, inline starts +0x26.
// retains retail selection policy and does not add controller mappings.
// Byte offsets preserve the pooled-string storage and original relocation addends.
enum CommonDialogStringsOffset {
    kCommonDialogStrings_common_dialogue_spd = 0x14, // common_dialogue.spd
    kCommonDialogStrings_black_sprite_anm_b2d = 0x28, // black_sprite_anm.b2d
    kCommonDialogStrings_black_sprite = 0x3D, // black_sprite
    kCommonDialogStrings_FontRec = 0x4A, // FontRec
    kCommonDialogStrings_ui_button_ok_click = 0x52, // ui_button_ok_click
    kCommonDialogStrings_ui_button_cancel_click = 0x65, // ui_button_cancel_click
    kCommonDialogStrings_R_dlwin_l_over = 0x7C, // R_dlwin_l_over
    kCommonDialogStrings_window = 0x8B, // window
};

enum CommonDialogButton { kDialogOk = 0, kDialogCancel = 1, kDialogThirdAcceptAction = 2 };
enum CommonDialogResult { kDialogPending = 0, kDialogAccepted = 1, kDialogCancelled = 2 };
// Four calls are suppressed while the OLD countdown is positive; input resumes
// on the following call. Units are process calls, independent of refresh Hz.
enum CommonDialogTiming { kDialogInputDelayUpdates = 4, kDialogButtonCount = 3, kDialogLayoutElementCount = 6 };
// Layout map: 0 R_btn_decide_yes (OK/Yes), 1 R_btn_decide_no (Cancel/No),
// 2 R_btn_dl (the third action); 3/4/5 are paired hit-state elements at button+3.
// Kind scripts: 0 yes_no_L, 1 yes_no_M, 2 yes_no_S, 3/4 userdata_dialogue.
// Kind 3 clears states on Yes/No, kind 4 on all three; other kinds clear the
// third action. All paired states and script group 0x3EE are also cleared.
// Kind policy established by callers in file_io_menu_save_load_base.cpp:
// 3 = CFileIoMenu_ShowAcknowledgeDialog: disable Yes/No, leave third action
//     available; activation ID 2 maps to accepted result 1 in vfunc_2.
// 4 = CFileIoMenu_CreateButtonlessCommonDialog: disable all three; the file-I/O
//     state machine replaces/destroys it when the operation completes, rather
//     than asking the user for an answer. It cannot complete through button input.
// Both use the SAME userdata_dialogue script pointer (commonDialogKindScriptNames
// script selection and ConfigureButtonLayout here; no hidden fourth result is
// implied by the shared resource name. UI state clearing disables activation:
// Layout_SetElementPlayerFlagsAndState clears player flags as well as feedback.
// A later mod changing kind policy must keep script groups, hit pairs, activation
// IDs and the caller's completion/replacement path together.
enum CommonDialogKind { kDialogAcknowledgeKind = 3, kDialogButtonlessKind = 4 };
enum { kDialogPairedElementOffset = 3, kDialogAuxiliaryGroupId = 0x3EE };
enum CommonDialogOpacity { kDialogOpaque = 255, kDialogScriptOpaque = 0x80, kDialogColorBias = 0x100 };

#include "classes/WFixedFont.h"
struct DialogMessageManagerFields { char prefix[0x10]; WFixedFont* font; };
#include "classes/CBpCtrlHandle.h"
// CPangyaCommonDialog_dtor (0x80279E80): entry IDs at +0x14/+0x18 are
// deleted, owned layout/message/tag-expander pointers are released, and
// savedScriptIndex (+0x40) is restored. String storage starts at +0x24.
#include "classes/CMsgDraw.h"
#define AsCMsgDraw(p) ((CMsgDraw*)(p))
#include "classes/CPangyaCommonDialogFields.h"
enum CommonDialogStringOffset { kDialogTextOffset = (int)&((CPangyaCommonDialogFields*)0)->textFlags, kDialogInlineTextOffset = kDialogTextOffset + 2 };

#include "game/wstring.h"
#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u80279C64 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Free collected layout IDs; the unsigned subtraction preserves count-clear code.
    ~LocalIntVector_u80279C64() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

// Translation unit 0x80279C64-0x8027B374 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
extern "C" {

extern void* g_mouseCursor;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_8050A210[];
extern char commonDialogStrings[];
extern void* commonDialogButtonGroupNames[]; // commonDialogButtonGroupNames
extern float kFloat_neg1_CommonDialogCursor; // kFloat_neg1_CommonDialogCursor

void Layout_GetElementPosition(void* self, int index, void* point);
void Layout_SetElementPosition(void* self, int index, void* point);
void Layout_GetElementSize(void* self, int index, void* point);
void Layout_WriteElementRect(void* self, int index, void* in);
void CMouseCursor_MoveTo(void* arg, float f1, float f2, float f3);
void __ct__7CBpCtrlFv(void* self);
void CBpCtrl_dtor(void* self, int index);
void* __register_global_object(void* a, void* b, void* c);
void* IdMap_Lookup(void* self, int index);
void UIGroup_GetPosition(void* self, void* out);
void UIGroup_GetSourceSize(void* self, void* point);
void BpSlot_SetUserData(void* self, int x, int y);
void CPangyaCommonDialog_ConfigureButtonLayout(void* self);
void B2dGetInfoPtr(void* self, int* x, int* y);
void Text_SetPosition(void* self, void* point);
void CMsgDraw_SetLayoutSize(void* self, void* point);
extern double kSignedIntToDoubleBias_CommonDialog; // kSignedIntToDoubleBias_CommonDialog
void* BpObject_GetOrCreateNamedGroup(void* self, const char* name);
void* UIGroup_GetOrCreateById(void* self, int value);
void UIGroup_SetEnabled(void* self, int value);
void Layout_SetElementColor(void* self, int index, int value);
void BpCtrl_SetOpacity(void* self, int value);
void CMsgDraw_ResetTextState(void* self);
void CMsgDraw_ClearFormattingAndRevealAll(void* self);
void Text_SetContent(void* self, void* value);
void BpObject_TryInvokeCurrentCallback(void* self, int value);
void BpObject_Draw(void* self);
void CMsgDraw_UpdateReveal(void* self);
void Text_DrawGlyphs(void* self);
void Layout_DrawWithOpacity(void* self, float value);
extern void *s_commonDialogueResourcesReady;
extern signed char s_bpCtrlInitialized;
extern float kFloat_1_CommonDialog; // kFloat_1_CommonDialog
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_8050A210[];
extern void* g_chatMsg;
int Layout_PollLoadComplete(void* self);
int BpObject_PollLoad(void* self);
void WFixedFont_CreateMsgEcbResource(void* self, void* data, int a, int b);
void Layout_SetElementSecondaryState(void* self, int index, int value);
void CBpCtrlHandle_ReleaseEntry(void* self);
void CBpCtrlHandle_Dtor(void* self);
void* IdMap_Lookup(void* self, int index);
// Existing declarations above cover the registration helper and callback.
void WString_ReplaceRange(void* self, void* first, int count, int value, int a, int b);
void Text_ExpandTaggedText(void* self, void* out, void* value, int flag);
void appFreeUni__FPv(void* self);
void Layout_Destroy(void* self, int flag);
void CBpCtrl_DeleteEntryById(void* self, int handle);
void CBpCtrlHandle_SetScriptIndex(void* self, int value);
void __dl__FPv(void* self);
void CBpCtrlHandle_Dtor(void* self);
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleDtorRegistration_CommonDialog[]; // s_bpCtrlHandleDtorRegistration_CommonDialog
void* __register_global_object(void* a, void* b, void* c);
void Layout_Update(void* self);
int Layout_CollectActivated(void* self, void* out, int player);
void Layout_SetElementPlayerFlagsAndState(void* self, int index, int player);
void Layout_SetElementEnabled(void* self, int index, int player);
void Sound_PlayByName(void* self, const char* name, int d, int e, int f, int g, float a, float b, float c);
void Layout_Init(void* self, int value);
void InputManager_Init(void* self);
void InputManager_Dtor(void* self);
int InputManager_GetSelectedPlayer(void* self);
void Layout_InitStandard(void* self, int value, const char* a, int count, void* name);
void CBpCtrl_CreateEntry(void* self, const char* name, void* out, int value);
void CPangyaCommonDialog_FinishLoading(void* self);
void CPangyaCommonDialog_StartFade(void* self, int a, int b);
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char __vt__19CPangyaCommonDialog[];
extern char s_inputManagerDtorRegistration_CommonDialog[]; // s_inputManagerDtorRegistration_CommonDialog
extern char s_commonDialogLayoutElements[]; // six-element layout definition for the common dialog script
extern float kFloat_0_5_CommonDialogSound; // kFloat_0_5_CommonDialogSound
extern float kFloat_200_CommonDialogSound; // kFloat_200_CommonDialogSound
extern float kFloat_1_CommonDialog; // kFloat_1_CommonDialog
void* __nw__FUl(unsigned int size);
struct InitObj {
    char pad[0x5c];
    // Construct the owned dialog layout wrapper in allocated storage.
    InitObj(int n) { Layout_Init(this, n); }
};

// Advance fade/input delay, then translate OK/cancel button activation into a result.
// Requires loaded layout/message. Writes countdown, opacity/fading and result.
// Delay is decremented before fading/completion; negative counts keep falling.
// Fading returns 0 even on its final tick. Clamps occur only OUTSIDE [0,255],
// so landing exactly on an endpoint takes one more tick to clear fading.
// First activation consumes feedback and resets delay even for an unlisted ID.
// The local ID vector owns only its allocation; it is freed on every exit.
int CPangyaCommonDialog_UpdateFadeAndInput(void* self) {
    if (s_commonDialogueResourcesReady == 0) return 0;
    Layout_Update(AsCPangyaCommonDialogFields(self)->layout);
    int timer = AsCPangyaCommonDialogFields(self)->inputDelay;
    AsCPangyaCommonDialogFields(self)->inputDelay = timer - 1;
    if (timer > 0) return 0;
    if (AsCPangyaCommonDialogFields(self)->fading) {
        int nextOpacity = AsCPangyaCommonDialogFields(self)->opacity + AsCPangyaCommonDialogFields(self)->opacityStep;
        AsCPangyaCommonDialogFields(self)->opacity = nextOpacity;
        if (nextOpacity < 0) { AsCPangyaCommonDialogFields(self)->opacity = 0; AsCPangyaCommonDialogFields(self)->fading = 0; }
        if (AsCPangyaCommonDialogFields(self)->opacity > kDialogOpaque) { AsCPangyaCommonDialogFields(self)->opacity = kDialogOpaque;
        AsCPangyaCommonDialogFields(self)->fading = 0;
        }
        struct __vA610 { 
        virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void m1() DECOMP_LOCAL_VIRTUAL; virtual void m2() DECOMP_LOCAL_VIRTUAL; virtual void m3() DECOMP_LOCAL_VIRTUAL; virtual void m4(int) DECOMP_LOCAL_VIRTUAL;
        };
        ((__vA610*)self)->m4(AsCPangyaCommonDialogFields(self)->opacity & 0xff);
        return 0;
    }
    if (AsCPangyaCommonDialogFields(self)->result) return 1;
    LocalIntVector_u80279C64 activatedButtons = {0, 0, 0};
    if (Layout_CollectActivated(AsCPangyaCommonDialogFields(self)->layout, &activatedButtons, 0)) {
        int buttonIndex = *(int*)activatedButtons.items;
        if (buttonIndex == kDialogThirdAcceptAction || buttonIndex == kDialogOk) {
            Sound_PlayByName(g_soundManager, commonDialogStrings + kCommonDialogStrings_ui_button_ok_click, 0, 0, 0, 0,kFloat_0_5_CommonDialogSound,kFloat_200_CommonDialogSound,kFloat_1_CommonDialog);
            AsCPangyaCommonDialogFields(self)->result = kDialogAccepted;
        } else if (buttonIndex == kDialogCancel) {
            Sound_PlayByName(g_soundManager, commonDialogStrings + kCommonDialogStrings_ui_button_cancel_click, 0, 0, 0, 0,kFloat_0_5_CommonDialogSound,kFloat_200_CommonDialogSound,kFloat_1_CommonDialog);
            AsCPangyaCommonDialogFields(self)->result = kDialogCancelled;
        }
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, *(int*)activatedButtons.items, 0);
        Layout_SetElementEnabled(AsCPangyaCommonDialogFields(self)->layout, *(int*)activatedButtons.items + kDialogPairedElementOffset, 1);
        AsCPangyaCommonDialogFields(self)->inputDelay = kDialogInputDelayUpdates;
    }
    return 0;
}

// Wait for dialog resources, create/position text and temporarily release the global script handle.
// Construction-only: kind must be 0..4; loaded Bp groups and font must exist.
// Poll loops have no timeout/error exit. Creates owned message/tagExpander;
// clears owned text, saves the global script index, then releases that handle.
void CPangyaCommonDialog_FinishLoading(void* self) {
    if (s_commonDialogueResourcesReady) {} else { return; }
    // Phase 1: block until layout, background and dialog scripts are ready.
    while (!Layout_PollLoadComplete(AsCPangyaCommonDialogFields(self)->layout)) Layout_Update(AsCPangyaCommonDialogFields(self)->layout);
    void* node;
    void* iter;
    for (;;) {
        if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
        s_bpCtrlInitialized=1;
        } Sing_805DC5F0()=s_bpCtrl;
        }
        iter=IdMap_Lookup(Sing_805DC5F0(),AsCPangyaCommonDialogFields(self)->backgroundEntryId); if(BpObject_PollLoad(iter)) break;
    }
    for (;;) {
        if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
        s_bpCtrlInitialized=1;
        } Sing_805DC5F0()=s_bpCtrl;
        }
        iter=IdMap_Lookup(Sing_805DC5F0(),AsCPangyaCommonDialogFields(self)->dialogEntryId); if(BpObject_PollLoad(iter)) break;
    }
    if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
    s_bpCtrlInitialized=1;
    } Sing_805DC5F0()=s_bpCtrl;
    }
    node=IdMap_Lookup(Sing_805DC5F0(),AsCPangyaCommonDialogFields(self)->dialogEntryId);
    while (!BpObject_PollLoad(node)) {}
    extern char* commonDialogKindScriptNames[]; // commonDialogKindScriptNames
    extern void BpObject_StartScriptByName(void*, const char*, int);
    // Phase 2: choose the kind script and align buttons, then create owned text objects.
    BpObject_StartScriptByName(node,commonDialogKindScriptNames[AsCPangyaCommonDialogFields(self)->kind],1); // commonDialogKindScriptNames
    if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
    s_bpCtrlInitialized=1;
    } Sing_805DC5F0()=s_bpCtrl;
    }
    node=IdMap_Lookup(Sing_805DC5F0(),AsCPangyaCommonDialogFields(self)->backgroundEntryId);
    BpObject_StartScriptByName(node,commonDialogStrings + kCommonDialogStrings_black_sprite,1);
    CPangyaCommonDialog_ConfigureButtonLayout(self);
    void* obj;
    void* manager=g_chatMsg;
    void* mgrObj=((DialogMessageManagerFields*)manager)->font;
    WFixedFont_CreateMsgEcbResource(mgrObj,&AsCPangyaCommonDialogFields(self)->tagExpander,0,0);
    extern void* __nw__FUl(unsigned int);
    extern void CMsgDraw_Ctor(void*,void*);
    struct MsgObj {
        char pad[0x4bc];
        // Construct the owned message renderer against the borrowed font resource.
        MsgObj(void* arg) { CMsgDraw_Ctor(this,arg); }
    };
    obj=new MsgObj(((WFixedFont*)mgrObj)->messageResource);
    AsCPangyaCommonDialogFields(self)->message=obj;
    // Phase 3: erase the current UTF-16 span and attach the empty dialog-owned text.
    // See SetText for the byte-address iterator subtraction and allocator-token ABI.
    // Local map for the erase below (everything is a byte address / UTF-16 unit count):
    //   a      = start of current text (inline buffer at self+kDialogInlineTextOffset, or textData if heap-flagged)
    //   b      = current length in UTF-16 units (low 7 bits of the first flag byte when inline, else textLength)
    //   end    = a + 2*b, one past the last unit
    //   a2     = start again (re-derived after the length read; same value as a)
    //   d1     = end - a2 in bytes; count = d1/2 units (the sign-adjust shift is signed division by 2)
    //   diff2  = a2 - start = 0, i.e. the erase begins at index 0
    // Net effect: WString_ReplaceRange(text, index 0, count units, replace with nothing).
    // `bytes` doubles as scratch: fill = allocator token for the replace call (its value is unused);
    // pos[2] = dialog entry origin x,y; p[0..1] = text position, p[2..3] = layout size (see Phase 4).
    int a,b;
    if(!(AsCPangyaCommonDialogFields(self)->textFlags>>31)) { a=(int)(((char*)self + kDialogInlineTextOffset));
    b=(*(unsigned char*)&AsCPangyaCommonDialogFields(self)->textFlags)&0x7f;
    }
    else { a=(int)AsCPangyaCommonDialogFields(self)->textData; b=AsCPangyaCommonDialogFields(self)->textLength; }
    int end=a+b*2;
    int a2=(int)((!(AsCPangyaCommonDialogFields(self)->textFlags>>31))?(void*)(((char*)self + kDialogInlineTextOffset)):AsCPangyaCommonDialogFields(self)->textData);
    int d1=end-a2;
    int count=(((int)((unsigned int)d1>>31))+d1)>>1;
    int diff2=a2-(int)((!(AsCPangyaCommonDialogFields(self)->textFlags>>31))?(void*)(((char*)self + kDialogInlineTextOffset)):AsCPangyaCommonDialogFields(self)->textData);
    struct { int fill; volatile unsigned char pad; char unused[3]; int pos[2]; float p[4]; } bytes;
    *(unsigned char*)&bytes.fill=bytes.pad;
    WString_ReplaceRange((void*)((unsigned int)self + kDialogTextOffset), (void*)(diff2/2), count, 0, 0, (int)&bytes.fill);
    CMsgDraw_ResetTextState(AsCPangyaCommonDialogFields(self)->message); CMsgDraw_ClearFormattingAndRevealAll(AsCPangyaCommonDialogFields(self)->message);
    Text_SetContent(AsCPangyaCommonDialogFields(self)->message,(!(AsCPangyaCommonDialogFields(self)->textFlags>>31))?(void*)(((char*)self + kDialogInlineTextOffset)):AsCPangyaCommonDialogFields(self)->textData);
    // Phase 4: place text in the FontRec rectangle relative to the dialog script.
    bytes.pos[0]=0; bytes.pos[1]=0; B2dGetInfoPtr((*(void**)&AsCPangyaCommonDialogFields(self)->dialogEntryId),&bytes.pos[0],&bytes.pos[1]);
    *(int*)&bytes.p[0]=0; *(int*)&bytes.p[1]=0; *(int*)&bytes.p[2]=0; *(int*)&bytes.p[3]=0;
    // Lazy singleton: the first use constructs the static CBpCtrl, registers its destructor and publishes it
    // through Sing_805DC5F0(). The same init-guard pattern repeats below for the CBpCtrlHandle (Sing_805DC5F8()).
    if(!Sing_805DC5F0()){if(!s_bpCtrlInitialized){__ct__7CBpCtrlFv(s_bpCtrl);
    __register_global_object(s_bpCtrl,(void*)CBpCtrl_dtor,s_bpCtrlDtorRegistration_8050A210);
    s_bpCtrlInitialized=1;
    }Sing_805DC5F0()=s_bpCtrl;
    }
    node=IdMap_Lookup(Sing_805DC5F0(),AsCPangyaCommonDialogFields(self)->dialogEntryId);
    void* pane=BpObject_GetOrCreateNamedGroup(node,commonDialogStrings + kCommonDialogStrings_FontRec);
    UIGroup_GetPosition(pane,&bytes.p[0]);
    bytes.p[0]+=bytes.pos[0];
    bytes.p[1]+=bytes.pos[1];
    UIGroup_GetSourceSize(pane,&bytes.p[2]);
    Text_SetPosition(AsCPangyaCommonDialogFields(self)->message,&bytes.p[0]);
    CMsgDraw_SetLayoutSize(AsCPangyaCommonDialogFields(self)->message,&bytes.p[2]);
    CMsgDraw_ResetTextState(AsCPangyaCommonDialogFields(self)->message);
    CMsgDraw_ClearFormattingAndRevealAll(AsCPangyaCommonDialogFields(self)->message);
    Text_SetContent(AsCPangyaCommonDialogFields(self)->message,(!(AsCPangyaCommonDialogFields(self)->textFlags>>31))?(void*)(((char*)self + kDialogInlineTextOffset)):AsCPangyaCommonDialogFields(self)->textData);
    // Phase 5: clear secondary states, save the global script index, release its entry.
    // Ownership: savedScriptIndex receives a copy of the global handle's scriptIndex; the handle's entry is then
    // released, so the dialog keeps only the index value, not the handle.
    for(int i=0;i<6;i++) Layout_SetElementSecondaryState(AsCPangyaCommonDialogFields(self)->layout,i,0);
    if(!Sing_805DC5F8()){if(!s_bpCtrlHandleInitialized){s_bpCtrlHandle[0]=-1;
    s_bpCtrlHandle[1]=-1;
    __register_global_object(s_bpCtrlHandle,(void*)CBpCtrlHandle_Dtor,s_bpCtrlHandleDtorRegistration_CommonDialog); // s_bpCtrlHandleDtorRegistration_CommonDialog
    s_bpCtrlHandleInitialized=1;
    }Sing_805DC5F8()=s_bpCtrlHandle;
    }
    (*(void**)&AsCPangyaCommonDialogFields(self)->savedScriptIndex)=(*(void**)&((CBpCtrlHandle*)Sing_805DC5F8())->scriptIndex);
    if(!Sing_805DC5F8()){if(!s_bpCtrlHandleInitialized){s_bpCtrlHandle[0]=-1;
    s_bpCtrlHandle[1]=-1;
    __register_global_object(s_bpCtrlHandle,(void*)CBpCtrlHandle_Dtor,s_bpCtrlHandleDtorRegistration_CommonDialog); // s_bpCtrlHandleDtorRegistration_CommonDialog
    s_bpCtrlHandleInitialized=1;
    }Sing_805DC5F8()=s_bpCtrlHandle;
    }
    CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
    return;
}

// Move the dialog window, then realign button hit rectangles and message text.
void CommonDialog_SetPosition(void* self, int x, int y) {
    struct S { char pad[0x18]; void* dialogEntryId; void* messageRenderer; char pad2[4]; unsigned int flag; char pad3[4]; void* text; };
    S* s = (S*)self;
    struct { float p[4]; float pos[2]; float anchorPosition[2]; } buffers;
    *(int*)&buffers.anchorPosition[0] = 0;
    *(int*)&buffers.anchorPosition[1] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
        s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* node = IdMap_Lookup(Sing_805DC5F0(), (int)s->dialogEntryId);
    void* pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_R_dlwin_l_over);
    UIGroup_GetPosition(pane, buffers.anchorPosition);
    *(int*)&buffers.pos[0] = 0;
    *(int*)&buffers.pos[1] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
        s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), (int)s->dialogEntryId);
    pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_window);
    UIGroup_GetPosition(pane, buffers.pos);
    BpSlot_SetUserData(s->dialogEntryId, (int)((float)x - buffers.anchorPosition[0] - buffers.pos[0]), (int)((float)y - buffers.anchorPosition[1] - buffers.pos[1]));
    CPangyaCommonDialog_ConfigureButtonLayout(self);
    int coords[2] = {0, 0};
    B2dGetInfoPtr(s->dialogEntryId, &coords[0], &coords[1]);
    *(int*)&buffers.p[0] = 0;
    *(int*)&buffers.p[1] = 0;
    *(int*)&buffers.p[2] = 0;
    *(int*)&buffers.p[3] = 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
        s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    node = IdMap_Lookup(Sing_805DC5F0(), (int)s->dialogEntryId);
    pane = BpObject_GetOrCreateNamedGroup(node, commonDialogStrings + kCommonDialogStrings_FontRec);
    UIGroup_GetPosition(pane, &buffers.p[0]);
    buffers.p[0] = buffers.p[0] + (float)coords[0];
    buffers.p[1] = buffers.p[1] + (float)coords[1];
    UIGroup_GetSourceSize(pane, &buffers.p[2]);
    Text_SetPosition(s->messageRenderer, &buffers.p[0]);
    CMsgDraw_SetLayoutSize(s->messageRenderer, &buffers.p[2]);
    CMsgDraw_ResetTextState(s->messageRenderer); CMsgDraw_ClearFormattingAndRevealAll(s->messageRenderer);
    Text_SetContent(s->messageRenderer, !(s->flag >> 31) ? ((char*)self + kDialogInlineTextOffset) : s->text);
}

}
struct DialogPrefix {
    void** vt; void* a4; int f8, fc; void* f10; int f14, f18, f1c, tagExpanderStorage;
    // Initialize the non-string prefix before WString construction can unwind.
    DialogPrefix(void* arg) {
        CPangyaCommonDialogFields* fields = AsCPangyaCommonDialogFields(this);
        fields->kind = (int)arg;
        fields->vtbl = __vt__19CPangyaCommonDialog;
        fields->result=0; fields->inputDelay=0; fields->layout=0;
        fields->backgroundEntryId=-1; fields->dialogEntryId=-1; fields->message=0;
    }
};
struct CPangyaCommonDialog {
    DialogPrefix prefix;
    WString text;
    char rest[0x14];
    CPangyaCommonDialog(void*, int);
};
// Construct dialog resources, acquire text storage and start its entrance fade.
// arg carries the dialog kind as a 32-bit ABI word; third is entrance-fade frames.
// Requires kind in 0..4 and live shared input/script/font services. This constructor
// owns layout/message/tag-expander/script entries; initial tagExpander storage is
// populated by FinishLoading rather than zeroed in the prefix constructor.
// Partial-construction contract: prefix and WString are real subobjects; layout,
// message and tagExpander are raw owning pointers and BP entries are IDs. No
// guard calls CPangyaCommonDialog_dtor if this constructor throws. C++ unwinds
// WString/completed subobjects and a failing new-expression's own allocation;
// already published layout/BP entries/message/tagExpander can remain acquired.
// Normal dtor frees layout -> message -> tagExpander -> dialog/background entries
// -> restores savedScriptIndex -> text. It is for fully initialized objects only.
// FinishLoading has no failure status: polls can wait forever; a false global
// resources-ready gate returns before creating tagExpander/message or saving the
// global index, but this constructor STILL runs StartFade, which uses message.
// Thus ready services, loaded groups/font, and successful allocations are entry
// preconditions, not recoverable outcomes. Do not destroy a skipped/failed draft
// as if it were complete (tagExpander may still be indeterminate).
// Only FinishLoading's last phase saves the old global script INDEX then releases
// its entry; that release is not rolled back on later failure. The normal dtor
// restores an index, not the released entry/resource. Serialize dialogs sharing
// that global handle; nested/concurrent construction is not a supported transaction.
CPangyaCommonDialog::CPangyaCommonDialog(void* arg, int third) : prefix(arg) {
    void* self = this;
    CPangyaCommonDialogFields* s = AsCPangyaCommonDialogFields(self);
    s->opacity=kDialogOpaque; s->opacityStep=kDialogOpaque; s->fadeDuration=third; s->savedScriptIndex=-1;
    // Phase 1: allocate the owned input layout, then resolve its input-manager owner.
    void* newLayout = new InitObj(5);
    volatile void* inputManagerSnapshot = Sing_805DC5C8();
    // Both outcomes store newLayout. The self-comparison retains MWCC scheduling;
    // it is not a readiness test and does not select a fallback layout.
    s->layout = (inputManagerSnapshot == inputManagerSnapshot) ? newLayout : newLayout;
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) { InputManager_Init(s_gameInputManager);
        __register_global_object(s_gameInputManager,(void*)InputManager_Dtor,s_inputManagerDtorRegistration_CommonDialog); // s_inputManagerDtorRegistration_CommonDialog
        s_inputManagerInitialized=1;
        }
        Sing_805DC5C8()=s_gameInputManager;
    }
    Layout_InitStandard(s->layout,InputManager_GetSelectedPlayer(Sing_805DC5C8()),s_commonDialogLayoutElements,kDialogLayoutElementCount,commonDialogStrings + kCommonDialogStrings_common_dialogue_spd);
    if (!Sing_805DC5F0()) { if(!s_bpCtrlInitialized){__ct__7CBpCtrlFv(s_bpCtrl);
    __register_global_object(s_bpCtrl,(void*)CBpCtrl_dtor,s_bpCtrlDtorRegistration_8050A210);
    s_bpCtrlInitialized=1;
    } Sing_805DC5F0()=s_bpCtrl;
    }
    // Phase 2: own the background/dialog script entries before finishing their load.
    CBpCtrl_CreateEntry(Sing_805DC5F0(),commonDialogStrings + kCommonDialogStrings_black_sprite_anm_b2d,&s->backgroundEntryId,0);
    if (!Sing_805DC5F0()) { if(!s_bpCtrlInitialized){__ct__7CBpCtrlFv(s_bpCtrl);
    __register_global_object(s_bpCtrl,(void*)CBpCtrl_dtor,s_bpCtrlDtorRegistration_8050A210);
    s_bpCtrlInitialized=1;
    } Sing_805DC5F0()=s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(),commonDialogStrings,&s->dialogEntryId,0);
    CPangyaCommonDialog_FinishLoading(self);
    CPangyaCommonDialog_StartFade(self,0,0);
    CPangyaCommonDialog_StartFade(self,1,s->fadeDuration);
}
extern "C" {

// Release owned layouts/text/script entries and restore the previous global script index.
void* CPangyaCommonDialog_dtor(void* self, int deleting) {
    if (self != 0) {
        CPangyaCommonDialogFields* obj = AsCPangyaCommonDialogFields(self);
        obj->vtbl = __vt__19CPangyaCommonDialog;
        if (obj->layout) {
            Layout_Destroy(obj->layout, 1);
            obj->layout = 0;
        }
        if (obj->message) {
            if (obj->message) { struct __d2 { 
            virtual void d(int) DECOMP_LOCAL_VIRTUAL;
            }; ((__d2*)obj->message)->d(1); }
            obj->message = 0;
        }
        if (obj->tagExpander) {
            if (obj->tagExpander) { struct __d2 { 
            virtual void d(int) DECOMP_LOCAL_VIRTUAL;
            }; ((__d2*)obj->tagExpander)->d(1); }
            obj->tagExpander = 0;
        }
        BpCtrl_SetOpacity((*(void**)&obj->dialogEntryId), kDialogScriptOpaque);
        BpCtrl_SetOpacity((*(void**)&obj->backgroundEntryId), kDialogScriptOpaque);
        if (obj->dialogEntryId != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), obj->dialogEntryId);
            obj->dialogEntryId = -1;
        }
        if (obj->backgroundEntryId != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), obj->backgroundEntryId);
            obj->backgroundEntryId = -1;
        }
        if (!Sing_805DC5F8()) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_CommonDialog); // s_bpCtrlHandleDtorRegistration_CommonDialog
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), obj->savedScriptIndex);
        if ((&AsCPangyaCommonDialogFields(self)->textFlags) && ((obj->textFlags >> 31) != 0)) appFreeUni__FPv(obj->textData);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Move the mouse cursor to the requested dialog button.
// index is a valid layout element (normally button 0..2); position is float xy
// in layout pixels. Duration -1 requests an immediate snap in CMouseCursor_MoveTo
// (mouse_cursor_hit_testing.cpp), including its pointer-position clamp and cursor-state reset.
void CPangyaCommonDialog_MoveCursorToButton(void* self, int index) {
    float buf[2];
    *(int*)buf = 0;
    *(int*)(buf + 1) = 0;
    Layout_GetElementPosition(AsCPangyaCommonDialogFields(self)->layout, index, buf);
    CMouseCursor_MoveTo(g_mouseCursor, buf[0], buf[1], kFloat_neg1_CommonDialogCursor); // kFloat_neg1_CommonDialogCursor
}

// Apply dialog opacity to layout controls, both script entries and message text.
// value is 0..255. Layout/message receive value-256 as the packed tint word;
// Bp entries receive floor((value&255)*128/255), whose unity is 128. Rebinds
// stored UTF-16 text and reveals it fully; it is not merely a GPU alpha setter.
void CPangyaCommonDialog_ApplyOpacity(void* self, int value) {
    int i;
    int n = value - kDialogColorBias;
    for (i = 0; i < kDialogLayoutElementCount; ++i)
        Layout_SetElementColor(AsCPangyaCommonDialogFields(self)->layout, i, n);
    int scaled = ((value & 0xff) << 7) / kDialogOpaque;
    BpCtrl_SetOpacity((*(void**)&AsCPangyaCommonDialogFields(self)->backgroundEntryId), scaled);
    BpCtrl_SetOpacity((*(void**)&AsCPangyaCommonDialogFields(self)->dialogEntryId), scaled);
    AsCMsgDraw(AsCPangyaCommonDialogFields(self)->message)->colorMultiplier = n;
    CMsgDraw_ResetTextState(AsCPangyaCommonDialogFields(self)->message);
    CMsgDraw_ClearFormattingAndRevealAll(AsCPangyaCommonDialogFields(self)->message);
    void* obj = AsCPangyaCommonDialogFields(self)->message;
    void* text = !( AsCPangyaCommonDialogFields(self)->textFlags >> 31) ? ((char*)self + kDialogInlineTextOffset) : AsCPangyaCommonDialogFields(self)->textData;
    Text_SetContent(obj, text);
}

// Draw the background and dialog scripts, reveal text and draw the button layout.
void CPangyaCommonDialog_Draw(void* self) {
    void* h;
    if (s_commonDialogueResourcesReady == 0) return;
    {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        h = IdMap_Lookup(Sing_805DC5F0(), AsCPangyaCommonDialogFields(self)->backgroundEntryId);
        BpObject_TryInvokeCurrentCallback(h, 0);
        BpObject_Draw(h);
    }
    {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        h = IdMap_Lookup(Sing_805DC5F0(), AsCPangyaCommonDialogFields(self)->dialogEntryId);
        BpObject_TryInvokeCurrentCallback(h, 0);
        BpObject_Draw(h);
    }
    CMsgDraw_UpdateReveal(AsCPangyaCommonDialogFields(self)->message);
    void* obj = AsCPangyaCommonDialogFields(self)->message;
    AsCMsgDraw(obj)->drawSuppressed = 0;
    Text_DrawGlyphs(obj);
    Layout_DrawWithOpacity(AsCPangyaCommonDialogFields(self)->layout, kFloat_1_CommonDialog); // kFloat_1_CommonDialog
}

#pragma opt_common_subs off
// Clear stored text, expand tags in the replacement and refresh the message renderer.
void CPangyaCommonDialog_SetText(void* self, void* taggedText) {
    // Phase 1: erase [0, old UTF-16 length). Inline data starts at object+0x26;
    // heap data uses textData and textLength. The repeated base reloads reconstruct
    // the original iterator subtraction: half2 is the start index (zero), half1
    // the erased code-unit count. Integer addresses below are byte addresses.
    unsigned int strFlag = AsCPangyaCommonDialogFields(self)->textFlags;
    int heapAllocated = (int)(strFlag >> 31);
    int lengthOrTextAddress; // Length first, then the text address for the second span calculation.
    int base;
    if (!heapAllocated) {
        lengthOrTextAddress = (*(unsigned char*)&AsCPangyaCommonDialogFields(self)->textFlags) & 0x7f;
        base = (int)(((char*)self + kDialogInlineTextOffset));
    } else {
        base = (*(int*)&AsCPangyaCommonDialogFields(self)->textData);
        lengthOrTextAddress = AsCPangyaCommonDialogFields(self)->textLength;
    }
    int end = base + lengthOrTextAddress * 2;
    int inlineText = !heapAllocated;
    base = inlineText ? (int)(((char*)self + kDialogInlineTextOffset)) : (*(int*)&AsCPangyaCommonDialogFields(self)->textData);
    int span1 = end - base;
    int half1 = span1 / 2;
    lengthOrTextAddress = inlineText ? (int)(((char*)self + kDialogInlineTextOffset)) : (*(int*)&AsCPangyaCommonDialogFields(self)->textData);
    int span2 = base - lengthOrTextAddress;
    int half2 = span2 / 2;
    // Allocator-token storage, not a text character. Its incoming byte is left
    // unspecified by this decompilation; do not use it as a fill/default value.
    volatile unsigned char pad;
    int fill;
    *(unsigned char*)&fill = pad;
    WString_ReplaceRange(&AsCPangyaCommonDialogFields(self)->textFlags, (void*)half2, half1, 0, 0, (int)&fill);
    // Phase 2: expand into an owning temporary, then expand again into stored text.
    // taggedText is borrowed NUL-terminated UTF-16; null means clear only.
    if (taggedText) {
        WString temp;
        Text_ExpandTaggedText(AsCPangyaCommonDialogFields(self)->tagExpander, &temp, taggedText, 0);
        void* src = !(temp.flags >> 31) ? (char*)&temp + 2 : (char*)temp.data;
        Text_ExpandTaggedText(AsCPangyaCommonDialogFields(self)->tagExpander, &AsCPangyaCommonDialogFields(self)->textFlags, src, 0);
    }
    // Phase 3: rebuild message formatting against the dialog-owned replacement.
    CMsgDraw_ResetTextState(AsCPangyaCommonDialogFields(self)->message);
    CMsgDraw_ClearFormattingAndRevealAll(AsCPangyaCommonDialogFields(self)->message);
    void* text = !(AsCPangyaCommonDialogFields(self)->textFlags >> 31) ? ((char*)self + kDialogInlineTextOffset) : AsCPangyaCommonDialogFields(self)->textData;
    Text_SetContent(AsCPangyaCommonDialogFields(self)->message, text);
}
#pragma opt_common_subs reset

// Set opacity immediately or begin an integer-step fade over process-call ticks.
// durationFrames must be nonnegative. Zero applies the endpoint immediately;
// otherwise opacityStep is integer +/-255/durationFrames (can truncate to zero).
// Nonzero duration keeps CURRENT opacity (no endpoint reset). Durations >255
// produce zero step and can stall forever. For positive step magnitude s from
// alpha a, fade-in completes when a+n*s>255; fade-out when a-n*s<0, not equality.
// Input delay and resource gates pause these ticks. The shared parameter/field
// spellings retain "Frames" for ABI/source compatibility; their unit is calls.
// Immediate fades still invoke virtual slot +0x18 with low-byte alpha; self owns
// all resources, and this function transfers none. Preserve division/sign/order.
void CPangyaCommonDialog_StartFade(void* self, int fadeIn, int durationFrames) {
    if (fadeIn != 0) {
        if (durationFrames == 0) {
            int notFading = 0;
            int opacityOrStep = kDialogOpaque;
            AsCPangyaCommonDialogFields(self)->fading = notFading;
            AsCPangyaCommonDialogFields(self)->opacity = opacityOrStep;
        } else {
            int opacityOrStep = kDialogOpaque / durationFrames;
            AsCPangyaCommonDialogFields(self)->fading = 1;
            AsCPangyaCommonDialogFields(self)->opacityStep = opacityOrStep;
        }
    } else {
        if (durationFrames == 0) {
            int opacityOrStep = 0;
            AsCPangyaCommonDialogFields(self)->fading = opacityOrStep;
            AsCPangyaCommonDialogFields(self)->opacity = opacityOrStep;
        } else {
            int opacityOrStep = -kDialogOpaque / durationFrames;
            AsCPangyaCommonDialogFields(self)->fading = 1;
            AsCPangyaCommonDialogFields(self)->opacityStep = opacityOrStep;
        }
    }
    int opacity = AsCPangyaCommonDialogFields(self)->opacity;
    struct __v6 { 
    virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void m1() DECOMP_LOCAL_VIRTUAL; virtual void m2() DECOMP_LOCAL_VIRTUAL; virtual void m3() DECOMP_LOCAL_VIRTUAL; virtual void m4(int) DECOMP_LOCAL_VIRTUAL;
    };
    ((__v6*)self)->m4(opacity & 0xff);
}

// Toggle the dialog window group if it exists.
// arg is forwarded unchanged to UIGroup_SetEnabled; no ownership transfer.
void CPangyaCommonDialog_SetWindowEnabled(void* self, int arg) {
    if (*(volatile void**)&Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* h = IdMap_Lookup(Sing_805DC5F0(), AsCPangyaCommonDialogFields(self)->dialogEntryId);
    void* style = BpObject_GetOrCreateNamedGroup(h, commonDialogStrings + kCommonDialogStrings_window);
    if (style != 0) {
        UIGroup_SetEnabled(style, arg);
    }
}

// Align three button/hit-state pairs and enable the buttons for the dialog kind.
// Required loaded dialog script and matching groups. Phase 1 maps each group
// position through the Bp entry's integer origin, copies float xy to both paired
// elements and rebuilds integer edge bounds. No bounds clipping is performed.
void CPangyaCommonDialog_ConfigureButtonLayout(void* self) {
    int left = 0, top = 0;
    B2dGetInfoPtr((*(void**)&AsCPangyaCommonDialogFields(self)->dialogEntryId), &left, &top);
    int i;
    for (i = 0; i < kDialogButtonCount; ++i) {
        float pos[2];
        *(int*)&pos[0] = 0; *(int*)&pos[1] = 0;
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* node = IdMap_Lookup(Sing_805DC5F0(), (int)(*(void**)&AsCPangyaCommonDialogFields(self)->dialogEntryId));
        void* pane = BpObject_GetOrCreateNamedGroup(node, (const char*)commonDialogButtonGroupNames[i]); // commonDialogButtonGroupNames
        UIGroup_GetPosition(pane, pos);
        UIGroup_SetEnabled(pane, 0);
        pos[0] = pos[0] + (float)left;
        pos[1] = pos[1] + (float)top;
        int point[2];
        point[0] = *(int*)&pos[0];
        point[1] = *(int*)&pos[1];
        Layout_SetElementPosition(AsCPangyaCommonDialogFields(self)->layout, i, point);
        Layout_GetElementSize(AsCPangyaCommonDialogFields(self)->layout, i, point);
        int rect[4];
        rect[0] = (int)pos[0];
        rect[1] = (int)pos[1];
        rect[2] = (int)(pos[0] + *(float*)&point[0]);
        rect[3] = (int)(pos[1] + *(float*)&point[1]);
        Layout_WriteElementRect(AsCPangyaCommonDialogFields(self)->layout, i, rect);
        int point2[2];
        point2[0] = *(int*)&pos[0];
        point2[1] = *(int*)&pos[1];
        Layout_SetElementPosition(AsCPangyaCommonDialogFields(self)->layout, i + kDialogPairedElementOffset, point2);
        Layout_GetElementSize(AsCPangyaCommonDialogFields(self)->layout, i + kDialogPairedElementOffset, point2);
        int rect2[4];
        rect2[0] = (int)pos[0];
        rect2[1] = (int)pos[1];
        rect2[2] = (int)(pos[0] + *(float*)&point2[0]);
        rect2[3] = (int)(pos[1] + *(float*)&point2[1]);
        Layout_WriteElementRect(AsCPangyaCommonDialogFields(self)->layout, i + kDialogPairedElementOffset, rect2);
    }
    // Phase 2: clear kind-dependent button states after all hit rectangles agree.
    int kind = AsCPangyaCommonDialogFields(self)->kind;
    if (kind == kDialogAcknowledgeKind) {
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 0, 0);
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 1, 0);
    } else if (kind == kDialogButtonlessKind) {
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 0, 0);
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 1, 0);
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 2, 0);
    } else {
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, 2, 0);
    }
    int r21;
    for (r21 = 3; r21 < 6; ++r21) {
        Layout_SetElementPlayerFlagsAndState(AsCPangyaCommonDialogFields(self)->layout, r21, 0);
    }
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    UIGroup_SetEnabled(UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), (int)(*(void**)&AsCPangyaCommonDialogFields(self)->dialogEntryId)), kDialogAuxiliaryGroupId), 0);
}

} // extern "C"
