// Tour hint/message UI: BP layout loading and opacity, shared text/reveal setup,
// confirm input, and teardown. Lookup: fade speed -> SetFadeStep (alpha/call),
// fade direction -> StartFadeIn/Out and UpdateFade, input -> SetInputMode and
// ProcessInput, text origin/content -> SetText. Layout group names/resources are
// tourInfoBaseGroupNames/tourHintLayoutResourceNames; reveal/page sounds and glyph
// scales are in the tourTipsWindowStrings / kTourMessage* constant companions.
// CMsgDraw implementation/contracts: msgdraw_text_rendering.cpp; input actions: Input_IsActionPressed.
extern "C" char s_messageDrawPairListDtorRegistration_TourMap[];
extern "C" char s_tourHintWindowDtorRegistration_802FE[];
extern "C" char s_tourTipsBpCtrlDtorRegistration[];
// Byte offsets into the existing packed string pool; no new string storage.
enum TourTipsStringOffset {
    kTourMessageRevealSound = 0x2E,
    kTourMessagePageSound = 0x37,
};

enum TourMessageConstants {
    kFadeComplete = 3,
    kFadeOut = 4,
    kFadeIn = 5,
    kOpaqueAlpha = 255,
    kConfirmAction = 4,
    kAlternateConfirmAction = 0x24,
    kAnyPlayer = -1,
    kInvalidLayoutEntry = -1,
    kConfirmOnlyInput = 0,
    kConfirmOrAlternateInput = 1
};
enum TourFadeTargets {
    kGlyphsOnly = 0,
    kGroupsAndGlyphs = 1,
    kHintGroupCount = 14,
    kHintTextCount = 4
};
#include "classes/CTourMessage.h"
#include "classes/CTourHintWindow.h"
#include "classes/CTourSubMessage.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CMsgDraw.h"
#include "classes/CMessageOwnerLayout.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpPlayer.h"

extern "C" {

#pragma use_lmw_stmw on

extern "C" const char __vt__12CTourMessage[];
extern "C" signed char s_bpPlayerInitialized;
extern "C" char s_bpPlayer[];
extern "C" void __dt__9CBpPlayerFv();
extern "C" char s_bpPlayerDtorRegistration_StoryMenu[];
extern "C" void __dl__FPv(void*);
extern "C" signed char s_tourHintWindowInitialized;
extern "C" char s_tourHintWindow[];
extern "C" char s_bpCtrl[];
extern "C" signed char s_bpCtrlInitialized;
extern "C" char s_tourHintWindowDtorRegistration_802FE[];
extern "C" char s_tourTipsBpCtrlDtorRegistration[];
extern "C" int tourHintLayoutResourceNames[2]; // Pointers to tour_hint.b2d and tour_minigm.b2d.
extern "C" void CTourHintWindow_dtor();
extern "C" void CBpCtrl_dtor();
extern "C" void* BpObject_GetOrCreateNamedGroup(void*, int);
extern "C" void UIGroup_SetOpacity(void*, int);
extern "C" int tourInfoBaseGroupNames[14];
extern "C" void* IdMap_Lookup(void*, int);
extern "C" int BpObject_PollLoad(void*);
extern "C" char __vt__15CTourSubMessage[];
extern "C" char s_messageDrawPairList[];
extern "C" signed char s_pointerPairListInitialized;
extern "C" char s_messageDrawPairListDtorRegistration_TourMap[];
extern "C" void PairVector_InitReserved(void*);
extern "C" void MessageDrawPairList_Dtor(); // PointerPairList_Dtor.
extern "C" void PointerPairList_RemoveByKey(void*, int);
extern "C" void Text_SetPosition(void*, void*);
extern "C" void Text_SetContent(void*, void*);
extern "C" char tourTipsWindowStrings[];
extern "C" float kFloat_0_5_TourMessage;
extern "C" float kFloat_200_TourMessageSound; // 200: kFloat_200_TourMessageSound.
extern "C" float kFloat_1_TourMessageSound;   // 1: kFloat_1_TourMessageSound.
extern "C" int g_soundManager;
extern "C" signed char s_inputManagerInitialized;
extern "C" char s_gameInputManager[];
extern "C" char
    s_tourMessageInputDtorRegistration[]; // Destructor registration storage for tourMessageInputDtorRegistration.
extern "C" char tourTipsWindowStrings[];
extern "C" void InputManager_Init(void*);
extern "C" void InputManager_Dtor();
extern "C" int Input_IsActionPressed(void*, int, int);
extern "C" void Sound_PlayByName(int, const char*, int, int, int, int, float, float, float);
extern "C" char s_bpPlayer[];
extern "C" char tourTipsWindowStrings[];
extern "C" void*
    kTourMessageGlyphScaleX;          // 1.0f glyph X scale; pointer-typed declaration preserves the existing load ABI.
extern void* kTourMessageGlyphScaleY; // 1.0f glyph Y scale; pointer-typed declaration preserves the existing load ABI.
extern "C" void __dt__9CBpPlayerFv();
extern "C" void Text_SetPosition(void*, void*);
extern "C" void Text_SetContent(void*, void*);

// Lazy singleton set-up the original code pasted inline at every use: if the global pointer is still null,
// construct the static object once (guard flag + destructor registration) and publish it in the pointer.
// After the call Sing_805DC670() holds the BpPlayer, Sing_805DC5F0() the BpCtrl, Singleton_TourHintWindow() the tour hint window and
// Sing_805DC650() the message-draw pointer-pair list.
static inline void EnsureBpPlayer() {
    if (!Sing_805DC670()) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_StoryMenu);
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = s_bpPlayer;
    }
}
static inline void EnsureBpCtrl(char* dtorRegistration) {
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, dtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}
static inline void EnsureHintWindow(char* dtorRegistration) {
    if (((int&)Singleton_TourHintWindow()) == 0) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, dtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((int&)Singleton_TourHintWindow()) = (int)s_tourHintWindow;
    }
}
static inline void EnsurePairList(char* dtorRegistration) {
    if (!((int&)Sing_805DC650())) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor, dtorRegistration);
            s_pointerPairListInitialized = 1;
        }
        ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
}

struct HintGroupNameTable {
    int values[14];
};
// Advance the hint opacity and apply it to BP groups and four text renderers.
// mode 1 also updates the 14 named BP groups; other modes touch only glyphs.
// With mode == 0 or active == 0, apply baseGlyphColor as-is to existing glyphs;
// hide the BP groups ONLY for mode kGroupsAndGlyphs (1), hence only if inactive.
// Mode zero leaves group opacity untouched and bypasses fade-state advancement.
// Return 1 on that bypass or when entering with fadeState == kFadeComplete;
// an update that reaches the endpoint returns 0 until the next call.
int CTourHintWindow::UpdateFade(float) {
    int result = 0;
    if (!(mode != 0 && active)) {
        // Bypass: mode 1 clears BP opacity; every mode applies baseGlyphColor to existing text.
        switch (mode) {
        case kGroupsAndGlyphs: {
            HintGroupNameTable values = *(HintGroupNameTable*)tourInfoBaseGroupNames;
            int* groupNameCursor = values.values;
            for (unsigned i = 0; i < kHintGroupCount; ++i, ++groupNameCursor) {
                int groupNameId = *groupNameCursor;
                EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
                void* obj = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), layoutIds[1]);
                if (obj) {
                    void* found = BpObject_GetOrCreateNamedGroup(obj, groupNameId);
                    if (found)
                        UIGroup_SetOpacity(found, 0);
                }
            }
        }
        default:
            break;
        case kGlyphsOnly:
            break;
        }
        for (int i = 0; i < kHintTextCount; ++i)
            if (items[i])
                ((CMsgDraw*)((char*)items[i]))->SetAllGlyphColors(baseGlyphColor);
        return 1;
    }
    // Advance one opacity step (per call, not scaled by elapsed time).
    int fade = fadeState;
    switch (fade) {
    case kFadeComplete:
        result = 1;
        break;
    case kFadeOut: {
        int delta = (int)opacity - (int)fadeStep;
        // For delta in [-255,255], the arithmetic-shift mask implements max(delta, 0).
        opacity = (unsigned char)(delta & (((-delta) & ~delta) >> 31));
        if (!opacity)
            fadeState = kFadeComplete;
        break;
    }
    case kFadeIn: {
        int value = (int)opacity + (int)fadeStep;
        opacity = (unsigned char)(value < kOpaqueAlpha ? value : kOpaqueAlpha);
        if (opacity == kOpaqueAlpha)
            fadeState = kFadeComplete;
        break;
    }
    default:
        break;
    }
    // Propagate the same alpha to groups and glyphs. Keep each lazy singleton
    // lookup at its use: resource lookup/creation can call into other game code.
    unsigned char amount = opacity;
    switch (mode) {
    case kGroupsAndGlyphs: {
        HintGroupNameTable values = *(HintGroupNameTable*)tourInfoBaseGroupNames;
        int* groupNameCursor = values.values;
        for (unsigned i = 0; i < kHintGroupCount; ++i, ++groupNameCursor) {
            int groupNameId = *groupNameCursor;
            EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
            void* obj = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), layoutIds[1]);
            if (obj) {
                void* found = BpObject_GetOrCreateNamedGroup(obj, groupNameId);
                if (found)
                    UIGroup_SetOpacity(found, amount);
            }
        }
    }
    default:
        break;
    case kGlyphsOnly:
        break;
    }
    for (int i = 0; i < kHintTextCount; ++i)
        if (items[i])
            ((CMsgDraw*)((char*)items[i]))->SetAllGlyphColors(baseGlyphColor | amount);
    return result;
}

// Empty hook with no side effects; its interface role remains unresolved.
void fn_802FF430() {}

// Empty hook with no side effects; its interface role remains unresolved.
void fn_802FF6F4() {}

// Replace both hint-window BP entries with newly requested resources.
void CTourHintWindow::RecreateLayoutEntries() {
    EnsureHintWindow(s_tourHintWindowDtorRegistration_802FE);
    // The singleton is read again here, after the lazy setup, as the original does.
    CTourHintWindow* self = (CTourHintWindow*)((int&)Singleton_TourHintWindow());
    for (int i = 0; i < 2; ++i) {
        if (self->layoutIds[i] != -1) {
            EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
            ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(self->layoutIds[i]);
            self->layoutIds[i] = -1;
        }
        EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))
            ->CreateEntry((const void*)tourHintLayoutResourceNames[i], &self->layoutIds[i], 0);
    }
}

// Wii pools these separate 12-byte destructor records: BP at +0,
// pointer-pair list at +0xC, input at +0x18, and hint window at +0x24.
enum {
    kRegPairListOffset = 0xC,
    kRegHintWindowOffset = 0x24
};
// A text renderer as the singleton holds it; slot 2 is its deleting destructor.
struct TextRendererSlots {
    virtual void Slot0();
    virtual void Slot1();
    virtual void Destroy(int);
};
// Release the singleton's four text renderers and its two BP layout entries, then delete
// this object when flags > 0. Each renderer leaves the message-draw list before it is destroyed.
void* CTourHintWindow::ReleaseSingletonResources(int flags) {
    char* dtorRegistrationBase = (char*)s_tourTipsBpCtrlDtorRegistration;
    if (this) {
        EnsureHintWindow((dtorRegistrationBase + kRegHintWindowOffset));
        CTourHintWindow* window = (CTourHintWindow*)((int&)Singleton_TourHintWindow());
        // Pass 1: unregister borrowed registry entries before deleting the owned text objects.
        window->state = 3;
        for (int i = 0; i < 4; ++i) {
            if (window->items[i]) {
                EnsurePairList((dtorRegistrationBase + kRegPairListOffset));
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)window->items[i]);
            }
            if (window->items[i]) {
                if (window->items[i]) {
                    ((TextRendererSlots*)window->items[i])->Destroy(1);
                }
                window->items[i] = 0;
            }
        }
        EnsureHintWindow((dtorRegistrationBase + kRegHintWindowOffset));
        // Pass 2: destructors may reenter, so the singleton is queried again before this pass.
        window = (CTourHintWindow*)((int&)Singleton_TourHintWindow());
        window->state = 3;
        for (int i = 0; i < 4; ++i) {
            if (window->items[i]) {
                EnsurePairList((dtorRegistrationBase + kRegPairListOffset));
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)window->items[i]);
            }
            if (window->items[i]) {
                if (window->items[i]) {
                    ((TextRendererSlots*)window->items[i])->Destroy(1);
                }
                window->items[i] = 0;
            }
        }
        // Release both BP resource IDs after text ownership has been cleared.
        for (int i = 0; i < 2; ++i) {
            if (window->layoutIds[i] != -1) {
                EnsureBpCtrl(dtorRegistrationBase);
                ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(window->layoutIds[i]);
                window->layoutIds[i] = -1;
            }
        }
        if (flags > 0)
            __dl__FPv(this);
    }
    return this;
}

// Return true only after both hint-window BP entries finish loading.
bool CTourHintWindow::AreResourcesLoaded() {
    EnsureHintWindow(s_tourHintWindowDtorRegistration_802FE);
    // The singleton is read again here, after the lazy setup, as the original does.
    // Walks the object's words; the two ids are the words at 0x48 and 0x4C.
    int i = 0;
    int* p = (int*)((int&)Singleton_TourHintWindow());
    for (; i < 2; ++i, ++p) {
        if (p[18] == -1)
            return 0;
        EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
        if (!BpObject_PollLoad(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), p[18])))
            return 0;
    }
    return 1;
}

struct SubMessageChild {
    virtual void destroy(int);
};
// Unregister and delete the four child renderers and the owned message object.
void* CTourSubMessage::Destroy(int flags) {
    if (this) {
        vtable = __vt__15CTourSubMessage;
        for (int i = 0; i < 4; i++) {
            if (items[i]) {
                EnsurePairList(s_messageDrawPairListDtorRegistration_TourMap);
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)items[i]);
            }
            if (items[i]) {
                if (items[i]) {
                    ((SubMessageChild*)items[i])->destroy(1);
                }
                items[i] = 0;
            }
        }
        if (ownedMessage) {
            if (ownedMessage) {
                ((SubMessageChild*)ownedMessage)->destroy(1);
            }
            ownedMessage = 0;
        }
        if (flags > 0)
            __dl__FPv(this);
    }
    return this;
}

// Configure the shared renderer, set packed inline/heap text, and restart reveal tracking.
// self/text/style are required. style is not an owned style object: it points to
// two float coordinates {x,y}, copied into CMsgDraw origin and cursor by
// Text_SetPosition (msgdraw_text_rendering.cpp). These are message-layout drawing units, before any
// downstream viewport transform; this wrapper supplies no pixels/world conversion.
// The exact physical pixel scale depends on that downstream layout. Glyph scale
// is a separate dimensionless {1,1} pair. text is a packed wide-string view:
// flags>=0 => UTF-16 payload at +2, flags<0 => borrowed longText at +8; NUL required.
// Text_SetContent copies UTF-16 into renderer-owned string storage and builds its
// line/glyph state synchronously. text/style and the stack scale pair need only
// survive this call (no ownership transfer). The shared renderer survives it and
// is reused by ProcessInput/ApplyGlyphColorMask; another SetText replaces its text.
// Caller serializes access to this singleton and owns self. Calls can allocate;
// there is no status/rollback here: exceptions may leave renderer settings partly
// changed, and revealComplete is cleared only after Text_SetContent returns.
void CTourMessage::SetText(TourMessageText* text, void* style) {
    void* params[2] = {kTourMessageGlyphScaleX, kTourMessageGlyphScaleY};
    EnsureBpPlayer();
    void* obj = ((CMessageOwnerLayout*)Sing_805DC670())->GetOrCreateMessageDraw();
    ((CMsgDraw*)obj)->SetDefaultRevealSound(tourTipsWindowStrings + 0x2e);
    ((CMsgDraw*)obj)->SetColor((baseGlyphColor & 0xffffff00) | 0xff);
    ((CMsgDraw*)obj)->ResetTextState();
    ((CMsgDraw*)obj)->SetRevealStep(1, kFloat_0_5_TourMessage);
    ((CMsgDraw*)obj)->SetGlyphScale((float*)params);
    Text_SetPosition(obj, style);
    void* ptr;
    if ((int)text->flags >= 0) {
        ptr = (char*)text + 2;
    } else {
        ptr = text->longText;
    }
    Text_SetContent(obj, ptr);
    revealComplete = 0;
}

// Set the opacity change applied on each fade update.
void CTourHintWindow::SetFadeStep(unsigned char value) {
    fadeStep = value;
}

// Set the hint window state used by its owner.
void CTourHintWindow::SetState(int value) {
    state = value;
}

struct TourMessageColorView {
    char padding00[0x10];
    int baseGlyphColor;
};

// Set the RGB color mask combined with reveal opacity.
void CTourMessage::SetBaseGlyphColor(int value) {
    TourMessageColorView* self = (TourMessageColorView*)this;
    self->baseGlyphColor = value;
}

struct TourMessageInputModeView {
    char padding00[0x14];
    int inputMode;
};

// Store input policy unchanged; no range checking or return status. Required self.
// Accepted active policies: 0 = action 4 only; 1 = action 4 or action 0x24, both
// queried for any player (-1). Constructor defaults to 0. Any other integer
// disables both confirm paths in ProcessInput (no input queries/page sound), but
// that consumer still records reveal completion at entry. allowRevealSkip only
// gates skipping an unfinished reveal; it never prevents completed-line advance.
// This is action selection, not a controller-device enumeration or debounce rate.
void CTourMessage::SetInputMode(int value) {
    TourMessageInputModeView* self = (TourMessageInputModeView*)this;
    self->inputMode = value;
}

// Start increasing opacity from fully transparent.
void CTourHintWindow::StartFadeIn() {
    opacity = 0;
    fadeState = kFadeIn;
}

// Start decreasing opacity from fully opaque.
void CTourHintWindow::StartFadeOut() {
    opacity = 0xff;
    fadeState = kFadeOut;
}

// Test the owner-visible hint-window state.
int CTourHintWindow::IsState(int value) {
    return state == value;
}

struct TourMessageBaseView {
    void* vtable;        // 0x0
    int revealComplete;  // 0x4
    char padding08[0x8]; // 0x8..0xF
    int baseGlyphColor;  // 0x10
    int inputMode;       // 0x14
};

// Initialize the renderer base with white RGB and default input/reveal state.
CTourMessage::CTourMessage() {
    TourMessageBaseView* self = (TourMessageBaseView*)this;
    self->vtable = (void*)__vt__12CTourMessage;
    self->baseGlyphColor = -0x100;
    self->revealComplete = 0;
    self->inputMode = 0;
}

// Optionally free base-message storage when called as a deleting destructor.
void* __dt__15TourMessageBaseFv(void* self, int flags) {
    if (self != 0 && flags > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Apply the base RGB color combined with the supplied glyph-color mask.
void CTourMessage::ApplyGlyphColorMask(int value) {
    EnsureBpPlayer();
    void* obj = ((CMessageOwnerLayout*)Sing_805DC670())->GetOrCreateMessageDraw();
    int field = ((TourMessageBaseView*)this)
                    ->baseGlyphColor; // 0x802FF260: CTourMessage::CTourMessage initializes the color at +0x10
    ((CMsgDraw*)((char*)obj))->SetAllGlyphColors(field | value);
}

struct TourMessageInputView {
    char padding00[4];
    int revealCompleteAtEntry;
    char padding08[0x0c];
    int inputMode;
};
// Original 0x802FEEA4: every input query first ensures the shared manager exists.
static inline void* TourMessage_InputManager() {
    if (!((int&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_tourMessageInputDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
    return (void*)((int&)GetInputManagerSlot());
}

// Handle reveal skipping and next-line confirmation; return one after the final line.
// The +4 latch records reveal completion observed at entry, even if input changes the renderer.
// allowRevealSkip gates only CMsgDraw::ClearFormattingAndRevealAll while revealing;
// input is still queried when it is zero. Once revealed, confirm advances regardless. Return 1
// only for a confirm on a completed reveal for which NextLine reports no line.
int CTourMessage::ProcessInput(int allowRevealSkip) {
    TourMessageInputView* self = (TourMessageInputView*)this;
    int result = 0;
    EnsureBpPlayer();
    void* obj = ((CMessageOwnerLayout*)Sing_805DC670())->GetOrCreateMessageDraw();
    if (!((CMsgDraw*)obj)->IsRevealComplete()) {
        // Revealing: confirmation may clear formatting; do not advance the line.
        if ((self->inputMode == kConfirmOnlyInput &&
             Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer)) ||
            (self->inputMode == kConfirmOrAlternateInput &&
             (Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer) ||
              Input_IsActionPressed(TourMessage_InputManager(), kAlternateConfirmAction, kAnyPlayer)))) {
            if (allowRevealSkip)
                ((CMsgDraw*)obj)->ClearFormattingAndRevealAll();
        }
        self->revealCompleteAtEntry = 0; // 0x802FEEA4: reveal state at +4
    } else {
        // Revealed: advance once, restart reveal timing, and play the page sound.
        if ((self->inputMode == kConfirmOnlyInput &&
             Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer)) ||
            (self->inputMode == kConfirmOrAlternateInput &&
             (Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer) ||
              Input_IsActionPressed(TourMessage_InputManager(), kAlternateConfirmAction, kAnyPlayer)))) {
            if (!((CMsgDraw*)obj)->NextLine())
                result = 1;
            ((CMsgDraw*)obj)->SetRevealStep(0, kFloat_0_5_TourMessage);
            Sound_PlayByName(g_soundManager, tourTipsWindowStrings + kTourMessagePageSound, 0, 0, 0, 0,
                             kFloat_0_5_TourMessage, kFloat_200_TourMessageSound, kFloat_1_TourMessageSound);
        }
        self->revealCompleteAtEntry = 1;
    }
    return result;
}

} // extern "C"
