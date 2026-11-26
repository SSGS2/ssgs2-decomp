// The loading screen: tip pages with a continue prompt, the background behind them, and
// the message file each tip is read from. CLoadingDraw is the main class; CMsgDraw draws the text.
// The end of the file holds the flash-based file move, CFileIoAccessMoveFlash.

#include <decomp/local_class.h>
// Loading / tip pages, confirmation controls and viewport-sized background drawing.
// CLoadingDraw owns its CMsgDraw and resource wrappers; CBpCtrl owns registered
// BP entries. WFixedFont/message resources supply text; FadeState owns the capture.
// Shared layouts: CLoadingDraw, CMsgDraw, WFixedFont; pool: the data section.
// Modder lookup:
// - Widescreen background: CLoadingDraw::DrawBackgroundBpAndMessage sizes a four-vertex quad from
//   Engine_GetViewport, in viewport-local pixels (top left, +X right,+Y down).
//   It does not add viewport x/y. kFloat_0_0009765625_LoadingDraw=1/1024 and
//   kLoadingBackgroundVPerPixel=1/512 convert pixel width/height to U/V;
//   the V alias below keeps the shared scalar's legacy ABI spelling until rename.
// - Tip placement/wrapping: SelectAndShowMessage reads messageGroup's position
//   and source size from loadingDrawStrings' BP resource, then sets CMsgDraw.
//   Edit the layout group and text bounds together when moving the tip box.
// - Frame rate: kLoadingFadeUpdateCount=30 is a FadeState update-call count;
//   kLoadingPulseAlphaStepPerUpdate=12 moves the prompt's triangular alpha phase
//   on each UpdateMessagePagingAndContinue call. CMsgDraw::UpdateReveal and BP callbacks run in DrawBackgroundBpAndMessage,
//   so render-call frequency can affect text reveal separately from input updates.
// - Controller: action IDs 0/1/4 mean previous/next/confirm in Input_IsActionPressed;
//   -1 requests its existing controller-selection policy, not a hardware button ID.
// Preserve readiness/confirmation gates, previous-before-next input priority,
// singleton registration, RNG consumption (one rand only for built-in tip choice),
// background/BP/text draw order and temporary-string cleanup on exceptions.
// Unified transition map (input/update and draw are separate clocks):
// BeginLoadingScreen creates message resources, selects text, sets page index=0, configures
// arrows/prompts and suspends cursor drawing. The owner supplies readyToContinue.
// UpdateMessagePagingAndContinue: fade state 3 -> return 0 before input, paging or pulse changes.
// Otherwise previous (when index>0) has priority over next (when a next line
// exists); each move resets formatting, updates arrows and plays one page sound.
// Paging is processed even while readyToContinue==0. Then:
//   !ready -> pulse; ready + confirmation + no confirm input -> pulse;
//   ready + (!confirmation || confirm input) -> snap opaque, start 30-call
//   decreasing fade, resume cursor drawing, return 1 (close accepted).
// Pulse adds 12, wraps values>255 to -255, publishes abs(phase), requests the
// pending fade transition and returns 0. It does not run on the accepted-close path.
// DrawBackgroundBpAndMessage draws background, invokes/draws BP, then advances CMsgDraw reveal and
// draws glyphs whenever messageDraw exists, including while the update is fade-gated.
// Thus return 1 means acceptance, state 3 means fading, and neither proves reveal
// or fade completion. Page navigation and draw-driven reveal must stay separate.
#include "classes/WFixedFont.h"
#define LOADING_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
enum LoadingAction {
    kLoadingPreviousPage = 0,
    kLoadingNextPage = 1,
    kLoadingConfirm = 4
};
// Constants for the loading screen. kLoadingPageGroupId is the layout group id of the tip-page UI group;
// kLoadingTextColor is the message text colour (packed 0xRRGGBBAA). kUiQuadFormat / kUiTriangleStrip /
// kUiVertexStride are the vertex-format flags, primitive type and per-vertex byte size passed to the
// engine's immediate-mode draw call for the loading background. Consumer contract:
// VertexFormat_BuildDeclaration (gtl_texture_stage_tev_setup.cpp) decodes 5 as float3 position,
// 0xF000 as packed four-byte color, 0x10000 as float2 UV; stride is 12+4+8=24.
// Engine_ApplyStateBeforeDraw (gx_renderer_state_and_resources.cpp) recognizes low nibble 5 as SCREEN-SPACE
// position with viewport-based orthographic projection, unlike world variant1.
// This caller emits 0..W / 0..H, without adding viewport x/y. Its full-background
// placement agrees with that projection at the usual zero viewport origin;
// nonzero-origin mods must consider the projection bounds and quad together.
// Keep format, BackgroundVertex layout, template and stride together. 0x98 is
// GX_TRIANGLESTRIP; GX_GetPrimitiveCount converts four vertices to two triangles
// for DrawPrimitiveUP. The immediate draw borrows the stack quad for submission.
enum {
    kLoadingFadeDecreasing = 3,
    kLoadingFadeUpdateCount = 30,
    kLoadingTipCount = 50,
    kLoadingOpacityMax = 255,
    kLoadingPulseAlphaStepPerUpdate = 12,
    kLoadingPageGroupId = 0x3f5,
    kLoadingTextColor = 0x32326eff,
    kUiQuadFormat = 0x11f005,
    kUiTriangleStrip = 0x98,
    kUiVertexStride = 0x18
};
struct LoadingTextFactoryView {
    char prefix[0x10];
    WFixedFont* font;
};
struct LoadingStrings {
    char courseArchive[11];
    char layout[12];
    char loadingScript[8];
    char continuePrompt[14];
    char previousPage[10];
    char nextPage[12];
    char pageChangeHint[13];
    char greenLoadingIndicator[16];
    char blueLoadingIndicator[15];
    char pageChangeSound[15];
    char confirmSound[19];
    char messageGroup[6];
};
#include "classes/CMsgDraw.h"
// Borrow message-draw fields; CLoadingDraw keeps the renderer alive through drawing.
static inline CMsgDraw* AsCMsgDraw(void* p) {
    return (CMsgDraw*)p;
}
extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Arm cleanup for a fully constructed temporary string.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Disarm after explicit freeing or transfer of the allocation.
    void release() {
        active = false;
    }
    // Release heap storage on unwinding; inline storage belongs to the string itself.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u))
            appFreeUni__FPv((void*)words[2]);
    }
};

#include "game/wstring.h"
namespace loading_string_detail {
extern "C" unsigned int strlen(const char*);
extern "C" int String_ReserveCapacity(void*, int);
extern "C" int String_ReplaceRange(void*, int, int, const char*, const char*, void*);
}
namespace std {
template <> class basic_string<char, char_traits<char>, app::app_allocator<char> > {
public:
    unsigned int flags;
    unsigned int length;
    char* data;
    // Construct an empty inline byte string.
    basic_string() : flags(0), length(0), data(0) {}
    // Copy the borrowed C string; count receives its byte length for ABI-compatible scratch use.
    basic_string(unsigned int text, int& count) {
        struct Slot {
            volatile char value;
            char pad[3];
        } allocator, copy;
        const char* source = (const char*)text;
        flags = 0;
        length = 0;
        data = 0;
        count = loading_string_detail::strlen(source);
        loading_string_detail::String_ReserveCapacity(this, count);
        copy.value = allocator.value;
        loading_string_detail::String_ReplaceRange(this, 0, 0, source, source + count, &copy);
    }
    // Release an owned heap buffer; inline bytes need no separate free.
    ~basic_string() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the active representation's byte length.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow a null-terminated pointer valid until this string is modified or destroyed.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};
}
typedef std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > NString;

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadingDraw.h"
#include <decomp/cw_bitfields.h>
#include "classes/CPackTextureResource.h"
#include "classes/CMouseCursor.h"
#include "classes/CFileIoAccessMove.h"
#include "classes/CFileIoAccessMoveFlash.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"

// Borrow the loading-screen layout without changing its resource ownership.
static inline CLoadingDraw* AsCLoadingDraw(void* p) {
    return (CLoadingDraw*)p;
}

extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u802877E8 {
    const unsigned int* words;
    bool active;
    // Arm the common heap-string guard for a completed narrow or wide string.
    explicit StringUnwindCleanup_u802877E8(const void* string) : words((const unsigned int*)string), active(true) {}
    // Hand cleanup back to the explicit normal-return path.
    void release() {
        active = false;
    }
    // Free only an armed heap-backed string during exceptional scope exit.
    ~StringUnwindCleanup_u802877E8() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

extern float kFloat_6_2831855;
extern float s_loadingDrawTwoPi;
extern NString s_loadingDrawStringStorage;
extern char s_bpCtrlDtorRegistration_LoadingDraw[];
extern char s_bpCtrl[];
extern char s_loadingFadeDtorRegistration[];
extern char __vt__12CLoadingDraw[];
extern char __vt__22CFileIoAccessMoveFlash[];
extern char loadingDrawStrings[];
extern int kLoadingTipMessageTags[];
extern signed char s_bpCtrlInitialized;
extern signed char s_fadeOverlayInitialized;
extern int s_loadingDrawRequiresConfirmation;
extern int s_loadingMessageEntryIndex;
extern char s_fadeOverlay[];

int CSharedFilePtr_GetLoadedBuffer(void*);
void CBpCtrl_dtor(void*);
void __dl__FPv(void*);
void* IdMap_Lookup(void*, int);
void UIGroup_SetEnabled(void*, int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void FadeState_StartFadeIn(void*, int);
void FadeState_StartFadeOut(void*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void UIGroup_GetPosition(void*, void*);
void UIGroup_GetSourceSize(void*, void*);
void Text_SetPosition(void*, void*);
void Text_SetContent(void*, void*);
void appFreeUni__FPv(void*);
void MessageTable_ExpandEntryText(void*, void*, int, int);
int rand();
void Text_ExpandTaggedText(void*, void*, void*, int);
void Engine_GetViewport(void*);
void Engine_SetFlexibleVertexFormat(int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Text_DrawGlyphs(void*);
int String_ReserveCapacity(void*, int);
int String_ReplaceRange(void*, int, int, const char*, const char*, void*);
void* __nw__FUl(unsigned int);
void BpObject_StartScriptByName(void*, const char*, int);
void* UIGroup_GetOrCreateById(void*, int);
void WFixedFont_CreateMsgEcbResource(void*, void*, void*, int);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void InputManager_SuspendCursorDrawing(void*);
extern void* g_chatMsg;
extern signed char s_inputManagerInitialized;
extern void* g_mouseCursor;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_LoadingDraw[];
extern signed char s_inputManagerInitialized;
extern signed char s_fadeOverlayInitialized;
extern int g_soundManager;
extern float kFloat_0_5_LoadingSound, kFloat_200_LoadingDraw, kFloat_1_LoadingDraw;
int FadeState_GetState(void*);
int Input_IsActionPressed(void*, int, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void UIGroup_SetOpacity(void*, const char*);
void InputManager_ResumeCursorDrawing(void*);
void FadeState_RequestFramebufferCapture(void*);

// Page/confirm a ready loading screen and pulse its prompt; preserve fade and input ordering.
// self is required with resources created by BeginLoadingScreen. Returns 1 only on the
// accepted close request (starts a decreasing-alpha fade and restores cursor
// drawing); returns 0 while waiting OR while phase 3 is already decreasing.
// This phase-3 test does not mean the fade has completed. All group/font pointers
// are borrowed for the call. The prompt uses alpha units per update, not seconds.
int CLoadingDraw::UpdateMessagePagingAndContinue() {
    // Phase 1: obtain fade/input state; an active decreasing fade returns 0 immediately.
    // Action IDs are previous=0, next=1, confirm=4. messageLineIndex is zero-based.
    struct Registration {
        char input[12];
        char fade[12];
        char bp[12];
    };
    Registration* base = (Registration*)s_inputManagerDtorRegistration_LoadingDraw;
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (base->fade));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()) == kLoadingFadeDecreasing)
        return 0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, base->input);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }

    // Phase 2: previous takes priority over next; each successful move clears cached
    // formatting and recomputes BOTH arrow visibilities before playing the page sound.
    if (Input_IsActionPressed(GetInputManagerSlot(), kLoadingPreviousPage, -1) && messageLineIndex) {
        --messageLineIndex;
        ((CMsgDraw*)messageDraw)->PreviousLine();
        ((CMsgDraw*)messageDraw)->ClearFormattingAndRevealAll();
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base->bp));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* pageOrPromptGroup =
            BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                           (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, previousPage)));
        UIGroup_SetEnabled(pageOrPromptGroup, messageLineIndex != 0);
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base->bp));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        pageOrPromptGroup =
            BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                           (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, nextPage)));
        UIGroup_SetEnabled(pageOrPromptGroup, !((CMsgDraw*)messageDraw)->HasNoNextLine());
        Sound_PlayByName((void*)g_soundManager,
                         (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, pageChangeSound)), 0, 0, 0, 0,
                         kFloat_0_5_LoadingSound, kFloat_200_LoadingDraw, kFloat_1_LoadingDraw);
    } else {
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, base->input);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        if (Input_IsActionPressed(GetInputManagerSlot(), kLoadingNextPage, -1) &&
            !((CMsgDraw*)messageDraw)->HasNoNextLine()) {
            ++messageLineIndex;
            ((CMsgDraw*)messageDraw)->NextLine();
            ((CMsgDraw*)messageDraw)->ClearFormattingAndRevealAll();
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base->bp));
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            void* pageOrPromptGroup = BpObject_GetOrCreateNamedGroup(
                IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, previousPage)));
            UIGroup_SetEnabled(pageOrPromptGroup, messageLineIndex != 0);
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base->bp));
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            pageOrPromptGroup =
                BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                               (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, nextPage)));
            UIGroup_SetEnabled(pageOrPromptGroup, !((CMsgDraw*)messageDraw)->HasNoNextLine());
            Sound_PlayByName((void*)g_soundManager,
                             (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, pageChangeSound)), 0, 0, 0, 0,
                             kFloat_0_5_LoadingSound, kFloat_200_LoadingDraw, kFloat_1_LoadingDraw);
        }
    }

    // Phase 3: readyToContinue gates completion; confirmation mode waits for action 4.
    // Unconfirmed input falls through to prompt animation (0x80288148).
    do {
        if (readyToContinue) {
            if (s_loadingDrawRequiresConfirmation) {
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, base->input);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                if (!Input_IsActionPressed(GetInputManagerSlot(), kLoadingConfirm, -1))
                    break;
            }
            {
                if (s_loadingDrawRequiresConfirmation)
                    Sound_PlayByName((void*)g_soundManager,
                                     (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, confirmSound)), 0, 0,
                                     0, 0, kFloat_0_5_LoadingSound, kFloat_200_LoadingDraw, kFloat_1_LoadingDraw);
                if (!Sing_805DC608()) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (base->fade));
                        s_fadeOverlayInitialized = 1;
                    }
                    Sing_805DC608() = s_fadeOverlay;
                }
                FadeState_StartFadeIn(Sing_805DC608(), 0);
                if (!Sing_805DC608()) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (base->fade));
                        s_fadeOverlayInitialized = 1;
                    }
                    Sing_805DC608() = s_fadeOverlay;
                }
                FadeState_StartFadeOut(Sing_805DC608(), kLoadingFadeUpdateCount);
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, base->input);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                InputManager_ResumeCursorDrawing(GetInputManagerSlot());
                return 1;
            }
        }
    } while (0);

    // Phase 4: pulse the green prompt with abs(phase), wrapping +255 back to -255.
    int nextAlphaPhase = promptAlphaPhase + kLoadingPulseAlphaStepPerUpdate;
    promptAlphaPhase = nextAlphaPhase;
    if (nextAlphaPhase > kLoadingOpacityMax)
        promptAlphaPhase = -kLoadingOpacityMax;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (base->bp));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* pageOrPromptGroup = BpObject_GetOrCreateNamedGroup(
        IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
        (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, greenLoadingIndicator)));
    UIGroup_SetOpacity(pageOrPromptGroup, (const char*)abs(promptAlphaPhase));
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (base->fade));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_RequestFramebufferCapture(Sing_805DC608());
    return 0;
}
// Construct the owned loading-message renderer for the supplied text factory.
struct CMsgDrawStorage {
    char pad[0x4bc];
    // Initialize the owned message renderer with the borrowed font/resource factory.
    CMsgDrawStorage(void* p) {
        ((CMsgDraw*)this)->__ct(p);
    }
};
struct Base80287A08 {
    virtual void v1(int);
};

// Create the tip renderer, initialize page controls, hide the cursor, and start the loading fade.
// Admission: ctor completed, the readiness poll returned ready, BP entry/font factory valid;
// call ONCE. The message-file bytes (external mode) and font message archive are
// borrowed by the new ECB context and must outlive it. WFixedFont_CreateMsgEcbResource
// (pet_mesh_and_fixed_font.cpp) allocates/publishes the owned context then initializes borrowed tables;
// this function ignores its status and immediately uses the new renderer.
// Allocation/null/font/invalid-text failure has no local recovery or rollback.
// A throw after ECB publication leaves that member owned by self; a throw inside
// renderer construction does not publish messageDraw. The destructor releases
// whichever nonnull members were published: renderer, background, ECB, BP entry,
// pack; ResetMessage releases messageFile first. This is valid for the constructed
// CMsgEcbResource: its vtable (phase1_class_tables.cpp) selects
// CMsgEcbResource_dtor (mw_string_stream_and_text_tags.cpp), which ONLY conditionally frees the context
// allocation. It neither reads nor releases table/grouped-table pointers.
// Borrowed bytes must survive text selection/expansion and all other table USE,
// but need not survive this non-reading destructor. After ResetMessage, the ECB
// table pointer can dangle: no further SelectAndShowMessage or ECB lookup is
// admitted. Already expanded CMsgDraw text has its own copy. Re-entry
// overwrites members and leaks them; no partial-initialization success is reported.
// Cursor suspension occurs only after text/control setup; exceptions before it
// have not acquired a suspension. The normal accepted-close path resumes it.
void CLoadingDraw::BeginLoadingScreen() {
    void* r = 0;
    unsigned int base = (unsigned int)s_inputManagerDtorRegistration_LoadingDraw;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)(((void*)(base + 0x18))));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* a = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    BpObject_StartScriptByName(a, (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, loadingScript)), 1);
    void* p = messageFile;
    void* manager = ((LoadingTextFactoryView*)g_chatMsg)->font;
    if (p)
        WFixedFont_CreateMsgEcbResource(manager, &messageEcbResource, (void*)(long)CSharedFilePtr_GetLoadedBuffer(p),
                                        0);
    else
        WFixedFont_CreateMsgEcbResource(manager, &messageEcbResource, 0, 0);
    CMsgDrawStorage* obj = new CMsgDrawStorage(AsWFixedFont(manager)->messageResource);
    messageDraw = obj;
    SelectAndShowMessage();
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, ((void*)(base + 0x18)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    UIGroup_SetEnabled(
        BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                       (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, continuePrompt))),
        0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, ((void*)(base + 0x18)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    UIGroup_SetEnabled(
        BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                       (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, previousPage))),
        0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, ((void*)(base + 0x18)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    r = BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                       (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, nextPage)));
    UIGroup_SetEnabled(r, ((CMsgDraw*)messageDraw)->HasNoNextLine() == 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, ((void*)(base + 0x18)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    r = BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                                       (loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, pageChangeHint)));
    UIGroup_SetEnabled(r, ((CMsgDraw*)messageDraw)->HasNoNextLine() == 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, ((void*)(base + 0x18)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    r = UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), kLoadingPageGroupId);
    UIGroup_SetEnabled(r, ((CMsgDraw*)messageDraw)->HasNoNextLine() == 0);
    messageLineIndex = 0;
    promptAlphaPhase = kLoadingOpacityMax;
    readyToContinue = 0;
    unk_38 = 0;
    // Reserved loading state; only its reset is established here.
    CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, (void*)(base + 0));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    InputManager_SuspendCursorDrawing(GetInputManagerSlot());
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, ((void*)(base + 0xc)));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeIn(Sing_805DC608(), 0);
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, ((void*)(base + 0xc)));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeOut(Sing_805DC608(), kLoadingFadeUpdateCount);
    p = texturePack;
    if (p) {
        if (p) {
            ((Base80287A08*)p)->v1(1);
        }
        texturePack = 0;
    }
}
struct FreeableView {
    virtual void free(int);
};
struct VTable10 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
};
// Release the message renderer, background, ECB resource, script entry, and texture pack.
CLoadingDraw::~CLoadingDraw() {
    // Plain C: reset the vtable, run vfunc 8 of the base, then free each owned object
    // (message renderer, background pane, ECB resource, texture pack) and null its field,
    // and delete the bp-controller entry. The repeated "p != 0" tests are compiler-matching noise.
    *(void**)this = (void*)__vt__12CLoadingDraw;
    ((VTable10*)this)->v8();
    void* p = messageDraw;
    if (p != 0) {
        if (p != 0) {
            ((FreeableView*)p)->free(1);
        }
        messageDraw = 0;
    }
    p = backgroundPane;
    if (p != 0) {
        if (p != 0) {
            ((FreeableView*)p)->free(1);
        }
        backgroundPane = 0;
    }
    p = messageEcbResource;
    if (p != 0) {
        if (p != 0) {
            ((FreeableView*)p)->free(1);
        }
        messageEcbResource = 0;
    }
    if (bpEntryId != -1) {
        // Lazily create the global CBpCtrl singleton (Sing_805DC5F0 = its instance pointer), then drop our entry.
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    p = texturePack;
    if (p != 0) {
        if (p != 0) {
            ((FreeableView*)p)->free(1);
        }
        texturePack = 0;
    }
}

// Mark loading complete and switch loading indicators to the confirmation prompt when required.
void CLoadingDraw::MarkReadyAndShowContinuePrompt() {
    if (readyToContinue != 0)
        return;
    readyToContinue = 1;
    if (s_loadingDrawRequiresConfirmation == 0)
        return;

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* r = BpObject_GetOrCreateNamedGroup(
        IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
        (void*)((loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, continuePrompt))));
    UIGroup_SetEnabled(r, 1);

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    r = BpObject_GetOrCreateNamedGroup(
        IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
        (void*)((loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, greenLoadingIndicator))));
    UIGroup_SetEnabled(r, 0);

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    r = BpObject_GetOrCreateNamedGroup(
        IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
        (void*)((loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, blueLoadingIndicator))));
    UIGroup_SetEnabled(r, 0);

    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_loadingFadeDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeIn(Sing_805DC608(), 0);

    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_loadingFadeDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeOut(Sing_805DC608(), kLoadingFadeUpdateCount);
}

// Release the external message file and reset shared loading-message selection.
// Called as a virtual function at the START of CLoadingDraw::~CLoadingDraw. This ends table use,
// not ECB allocation lifetime: the later CMsgEcbResource_dtor only frees self
// (mw_string_stream_and_text_tags.cpp), so it never dereferences the now-invalid borrowed table address.
// Caller must quiesce ECB consumers first; do not reuse messageEcbResource for a
// lookup after reset, and do not infer that another registry reference keeps
// file bytes alive. Reset does not null the ECB pointer or synchronize readers.
void CLoadingDraw::ResetMessage() {
    void* p = messageFile;
    if (p != 0) {
        CSharedFileHandle::Destroy(p, 1);
        messageFile = 0;
    }
    unsigned int v = s_loadingDrawStringStorage.flags;
    s_loadingDrawRequiresConfirmation = 0;
    if (!(v >> 31)) {
        ((unsigned char*)&s_loadingDrawStringStorage)[1] = 0;
        struct Flag {
            CW_BITS2(unsigned char hi : 1, unsigned char lo : 7)
        };
        ((Flag*)&s_loadingDrawStringStorage)->lo = 0;
    } else {
        unsigned char* q = (unsigned char*)s_loadingDrawStringStorage.data;
        q[0] = 0;
        s_loadingDrawStringStorage.length = 0;
    }
    s_loadingMessageEntryIndex = -1;
}

// Poll message, background, BP and texture resources; return one only when all are ready.
// Despite the name this may BLOCK inside one call: the pack loop below has no
// yield, timeout, cancellation or error exit. IsLoadFinished only tests whether
// its pending path is empty (pack_texture_resource_loading.cpp); PollLoad returns pending until shared file
// bytes appear, then attempts archive loading, clears the path and releases file.
// Progress therefore requires the asynchronous file service to run independently;
// an unavailable/failed file whose buffer stays null can spin forever. Caller
// must not hold a lock needed by that service. Pack completion does not prove all
// textures loaded: PollLoad ignores LoadArchiveEntries' failure result. If the loop
// ran even once, result remains 0 and the caller must poll again after completion.
// Message/background/BP pending states merely clear result; they do not skip this
// pack drain. No allocations transfer to the caller and no output arg is written.
int CLoadingDraw::PollResourcesLoaded() {
    int result = 1;
    if (s_loadingDrawRequiresConfirmation != 0) {
        void* p = messageFile;
        if (CSharedFilePtr_GetLoadedBuffer(p) == 0) {
            result = 0;
        }
    }
    void* p24 = backgroundPane;
    if (p24 != 0) {
        struct V6 {
            virtual int a() DECOMP_LOCAL_VIRTUAL;
            virtual int b() DECOMP_LOCAL_VIRTUAL;
            virtual int c() DECOMP_LOCAL_VIRTUAL;
            virtual int d() DECOMP_LOCAL_VIRTUAL;
            virtual int e() DECOMP_LOCAL_VIRTUAL;
        };
        if (!((V6*)p24)->e()) {
            result = 0;
        }
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0) {
        result = 0;
    }
    // 0x80287AE8-0x80287B00: keep the original branch order for CodeWarrior matching.
    while (texturePack != 0 && ((CPackTextureResource*)(texturePack))->IsLoadFinished() == 0) {
        ((CPackTextureResource*)texturePack)->PollLoad();
        result = 0;
    }
    return result;
}

// Position and style the tip, then expand the selected external message or one of 50 random tips.
// Despite the shared name, confirmation mode is deterministic: messageEcbResource
// plus s_loadingMessageEntryIndex select an external table entry. Otherwise rand()%50
// selects a built-in tag. The temporary expanded string is copied into messageDraw
// before its heap buffer is freed; no ownership of that temporary escapes.
// External-index contract: 0 <= s_loadingMessageEntryIndex < the active ECB's
// MessageContext_GetResourceData count. It is a signed word at 805DB6F8 (initial
// -1 in the data section); ResetMessage restores -1. A source/include search finds no
// producer of a valid index or nonzero confirmation flag in this revision: the
// implemented external-mode producer is absent. The only other named path writer
// found is CTask warmup's String_ReserveCapacity (game_main_loop_and_startup.cpp), which reserves storage
// without selecting a message; construction supplies an empty NString, flag=0,
// index=-1. Thus no ordinary source caller establishes an enabled configuration.
// Producer contract for a future enabling mod (requirements derived from the
// consumers, NOT a claim that an existing setter enforces them): on the serial
// loading lifecycle, finish assigning an owned NString path naming readable,
// compatible ECB table bytes and a valid entry index, then publish nonzero
// confirmation BEFORE CLoadingDraw_Ctor (pop_item_dialog_and_loading_draw.cpp). It snapshots the path into
// SharedFile_Create only when the flag is set. PollResourcesLoaded must see a
// nonnull handle and loaded bytes before BeginLoadingScreen creates the ECB/expands text.
// Keep path/index/flag coherent until ResetMessage resets all three; do not overlap
// loading instances, since these settings are process globals, not per-screen.
// No lock, input validation, failed-open recovery or runtime reconfiguration
// protocol is supplied. Setting only the flag with default index -1 is invalid.
// Invalid entry is NOT blank-text fallback: MessageTable_GetTextByIndex
// (mw_string_stream_and_text_tags.cpp) calls unchecked Table_GetEntry (endian_format_and_binary_table.cpp: base+20+index*stride),
// dereferences that entry and resolves its text offset. Negative/out-of-range
// indices can read outside the table. Unsupported table format alone returns a
// null text pointer; neither this call nor the expansion return is checked here.
void CLoadingDraw::SelectAndShowMessage() {
    // UIGroup_GetPosition/GetSourceSize write float XY/width-height pairs.
    // Keep word initialization/copies: these int fields carry IEEE-754 bits,
    // not integer coordinates. Text_SetPosition/SetLayoutSize read the floats.
    struct Pair {
        int firstFloatBits;
        int secondFloatBits;
    } position = {0, 0}, size = {0, 0};

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* tmp = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    void* messageGroup = BpObject_GetOrCreateNamedGroup(
        tmp, (void*)((loadingDrawStrings + LOADING_MEMBER_OFFSET(LoadingStrings, messageGroup))));
    UIGroup_GetPosition(messageGroup, &position);
    UIGroup_GetSourceSize(messageGroup, &size);
    Text_SetPosition(messageDraw, &position);
    ((CMsgDraw*)messageDraw)->SetLayoutSize((float*)&size);
    ((CMsgDraw*)messageDraw)->SetColor(kLoadingTextColor);

    WString text;
    if (s_loadingDrawRequiresConfirmation != 0) {
        MessageTable_ExpandEntryText(messageEcbResource, &text, s_loadingMessageEntryIndex, 0);
    } else {
        int tipIndex = rand() % kLoadingTipCount;
        Text_ExpandTaggedText(messageEcbResource, &text, (void*)(long)kLoadingTipMessageTags[tipIndex], 0);
    }

    ((CMsgDraw*)messageDraw)->ResetTextState();

    void* expandedText = ((text.flags >> 31) == 0) ? (void*)((char*)&text + 2) : (void*)text.data;
    Text_SetContent(messageDraw, expandedText);
    ((CMsgDraw*)messageDraw)->ClearFormattingAndRevealAll();
}

float s_loadingDrawTwoPi = kFloat_6_2831855;
NString s_loadingDrawStringStorage;

// Pool the data section: normalized U per pixel = 1/1024; V per pixel = 1/512.
// Larger scales tile/sample more of the texture at the same viewport size.
// These are texture-coordinate controls, not output-resolution controls.
extern float kFloat_0_0009765625_LoadingDraw;
extern float kLoadingBackgroundVPerViewportPixel; // historical name; loading background only
#define kLoadingBackgroundVPerPixel kLoadingBackgroundVPerViewportPixel
extern unsigned int kLoadingBackgroundQuadTemplate[];
extern char flashMovePathAndExceptionStrings[];
// Historical shared name: this pool starts with "fmem:" and is used below as a
// file-path prefix, not as reveal-buffer text.

// Draw viewport-local background, BP layout and revealed tip text; preserve callback/reveal-before-draw order.
// self is borrowed and must have a valid BP entry. backgroundPane/messageDraw may
// be null; those stages are skipped. The background's bind virtual method supplies
// its texture; this function changes vertex format and does not restore prior state.
// Keep the two UV scales independent of aspect/projection changes; see lookup above.
void CLoadingDraw::DrawBackgroundBpAndMessage() {
    // Phase 1: size the screen-space background in pixels; UV repeats use
    // width/1024 and height/512 from the original texture scale constants.
    if (backgroundPane) {
        unsigned int viewport[6];
        // Four XYZ/RGBA/UV vertices; bit-preserving template assignment fills all fields.
        struct BackgroundVertex {
            float x, y, z;
            unsigned int rgba;
            float u, v;
        };
        struct Blk24 {
            BackgroundVertex vertices[4];
        };
        typedef char BackgroundQuadSize[(sizeof(Blk24) == 4 * kUiVertexStride) ? 1 : -1];
        struct V5 {
            virtual void a() DECOMP_LOCAL_VIRTUAL;
            virtual void b() DECOMP_LOCAL_VIRTUAL;
            virtual void c() DECOMP_LOCAL_VIRTUAL;
            virtual void f() DECOMP_LOCAL_VIRTUAL;
        };
        Blk24 quad;
        Engine_GetViewport(viewport);
        quad = *(Blk24*)kLoadingBackgroundQuadTemplate;
        unsigned int viewportWidth = viewport[2];
        unsigned int viewportHeight = viewport[3];
        quad.vertices[1].x = (float)viewportWidth;
        quad.vertices[1].u = (float)viewportWidth * kFloat_0_0009765625_LoadingDraw;
        quad.vertices[2].y = (float)viewportHeight;
        quad.vertices[2].v = (float)viewportHeight * kLoadingBackgroundVPerPixel;
        quad.vertices[3].x = (float)viewportWidth;
        quad.vertices[3].y = (float)viewportHeight;
        quad.vertices[3].u = (float)viewportWidth * kFloat_0_0009765625_LoadingDraw;
        quad.vertices[3].v = (float)viewportHeight * kLoadingBackgroundVPerPixel;
        ((V5*)backgroundPane)->f();
        Engine_SetFlexibleVertexFormat(kUiQuadFormat);
        void* verts = GX_GetPrimitiveCount(4, kUiTriangleStrip);
        DrawPrimitiveUP(kUiTriangleStrip, verts, quad.vertices, kUiVertexStride);
    }
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadingDraw);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* r = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    BpObject_TryInvokeCurrentCallback(r, 0);
    BpObject_Draw(r);
    if (messageDraw) {
        ((CMsgDraw*)messageDraw)->UpdateReveal();
        AsCMsgDraw(messageDraw)->drawSuppressed = 0;
        Text_DrawGlyphs(messageDraw);
    }
}
#pragma push
#pragma use_lmw_stmw on
// Construct the base move operation and install its flash-specific interface.
inline CFileIoAccessMoveFlash::CFileIoAccessMoveFlash() {
    ((CFileIoAccessMove*)this)->__ct();
    vtbl = (void*)__vt__22CFileIoAccessMoveFlash;
}

// Prefix destination and source paths, create move access, and discard it on setup failure.
// Argument order is DESTINATION, SOURCE: CommitSaveFile/CommitBannerFile in
// flash_memory_menu_save.cpp pass the permanent filename first and the temporary filename second.
// The backing CFileIoFlash::MoveQualifiedPath (cfileio_flash_file_operations.cpp) calls FileIO_MoveFile
// with source,destination, reversing these accessor arguments at that boundary.
// Both are borrowed NUL-terminated relative flash paths; null is rejected.
// No normalization/prefix detection occurs: "fmem:" is prepended to BOTH paths
// (the source is assembled first). SetPathsAndStart (small_common_dialog.cpp) copies the two
// strings into firstPath=destination, secondPath=source before creating access.
// Its parameter names currently say source,destination; that shared spelling is
// a legacy name, not the file-move direction. All temporary/caller text may
// be released on return. Nonnull transfers the access object to the caller,
// released through its virtual deleting destructor (+0x14, argument 1).
// Nonnull means accepted setup, not completed move: poll IsFinished and the
// failure latch. Null input/allocation/rejected setup returns null and destroys
// any allocated access. A later move failure has no rollback guarantee here.
void* CFileIoAccessMoveFlash::Create(void* destinationPath, void* sourcePath) {
    CFileIoAccessMoveFlash* obj = new CFileIoAccessMoveFlash();
    if (!obj)
        return 0;
    unsigned int moveStarted;
    int textLength;
    if (!sourcePath || !destinationPath) {
        moveStarted = 0;
    } else {
        struct Slot {
            volatile char v;
            char pad[3];
        };
        // Allocator-tag slots are intentionally uninitialized; only volatile
        // byte copies are observed by the string ABI. Declaration order matters.
        Slot c7, c6, c5, c4, c3, c2, c1, c0;
        NString prefixedSource((unsigned int)flashMovePathAndExceptionStrings, textLength);
        textLength = prefixedSource.size();
        c3.v = c2.v;
        unsigned int len = strlen((const char*)sourcePath);
        String_ReplaceRange(&prefixedSource, textLength, 0, (const char*)sourcePath, (const char*)sourcePath + len,
                            &c3);
        NString prefixedDestination((unsigned int)flashMovePathAndExceptionStrings, textLength);
        textLength = prefixedDestination.size();
        c7.v = c6.v;
        String_ReplaceRange(&prefixedDestination, textLength, 0, (const char*)destinationPath,
                            (const char*)destinationPath + strlen((const char*)destinationPath), &c7);
        const char* sourceText = prefixedSource.c_str();
        const char* destinationText = prefixedDestination.c_str();
        moveStarted = ((CFileIoAccessMove*)obj)->SetPathsAndStart(destinationText, sourceText);
    }
    if (!moveStarted) {
        struct VF {
            virtual void isFinished() DECOMP_LOCAL_VIRTUAL;
            virtual void update() DECOMP_LOCAL_VIRTUAL;
            virtual void capability() DECOMP_LOCAL_VIRTUAL;
            virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
        };
        if (obj)
            ((VF*)obj)->destroy(1);
        return 0;
    }
    return obj;
}
#pragma pop

} // extern "C"
