// Tour hint/message UI: BP layout loading and opacity, shared text/reveal setup,
// confirm input, and teardown. Lookup: fade speed -> SetFadeStep (alpha/call),
// fade direction -> StartFadeIn/Out and UpdateFade, input -> SetInputMode and
// ProcessInput, text origin/content -> SetText. Layout group names/resources are
// tourInfoBaseGroupNames/tourHintLayoutResourceNames; reveal/page sounds and glyph
// scales are in the tourTipsWindowStrings / kTourMessage* constant companions.
// CMsgDraw implementation/contracts: msgdraw_text_rendering.cpp; input actions: Input_IsActionPressed.
// CTourMessage.h exposes +0x40 fadeState; the partial views below refer to the
// same objects (base message prefix versus extended hint-window fields).
extern "C" char s_messageDrawPairListDtorRegistration_TourMap[];
extern "C" char s_tourHintWindowDtorRegistration_802FE[];
extern "C" char s_tourTipsBpCtrlDtorRegistration[];
// Byte offsets into the existing packed string pool; no new string storage.
enum TourTipsStringOffset {
    kTourMessageRevealSound = 0x2E,
    kTourMessagePageSound = 0x37,
};

enum TourMessageConstants {
    kFadeComplete = 3, kFadeOut = 4, kFadeIn = 5,
    kOpaqueAlpha = 255, kConfirmAction = 4, kAlternateConfirmAction = 0x24,
    kAnyPlayer = -1, kInvalidLayoutEntry = -1,
    kConfirmOnlyInput = 0, kConfirmOrAlternateInput = 1
};
enum TourFadeTargets { kGlyphsOnly = 0, kGroupsAndGlyphs = 1,
                       kHintGroupCount = 14, kHintTextCount = 4 };
#include "classes/CTourMessage.h"
// Translation unit 0x802FE92C-0x802FF968 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"

extern "C" {

#pragma use_lmw_stmw on

extern "C" const char __vt__12CTourMessage[];
extern "C" signed char s_bpPlayerInitialized;
extern "C" char s_bpPlayer[];
extern "C" void CBpPlayer_Ctor(void*);
extern "C" void CBpPlayer_Dtor();
extern "C" char s_bpPlayerDtorRegistration_StoryMenu[];
extern "C" void* CTourMessage_GetOrCreateMessageDraw(void*);
extern "C" void CMsgDraw_SetAllGlyphColors(char*, int);
extern "C" void __dl__FPv(void*);
extern "C" signed char s_tourHintWindowInitialized;
extern "C" char s_tourHintWindow[];
extern "C" char s_bpCtrl[];
extern "C" signed char s_bpCtrlInitialized;
extern "C" char s_tourHintWindowDtorRegistration_802FE[];
extern "C" char s_tourTipsBpCtrlDtorRegistration[];
extern "C" int tourHintLayoutResourceNames[2]; // Pointers to tour_hint.b2d and tour_minigm.b2d.
extern "C" void __ct__15CTourHintWindowFv(void*);
extern "C" void CTourHintWindow_dtor();
extern "C" void __ct__7CBpCtrlFv(void*);
extern "C" void CBpCtrl_dtor();
extern "C" void CBpCtrl_CreateEntry(void*, int, void*, int);
extern "C" void* BpObject_GetOrCreateNamedGroup(void*, int);
extern "C" void UIGroup_SetOpacity(void*, int);
extern "C" int tourInfoBaseGroupNames[14];
extern "C" void CBpCtrl_DeleteEntryById(void*, int);
extern "C" void* IdMap_Lookup(void*, int);
extern "C" int BpObject_PollLoad(void*);
extern "C" char __vt__15CTourSubMessage[];
extern "C" char s_messageDrawPairList[];
extern "C" signed char s_pointerPairListInitialized;
extern "C" char s_messageDrawPairListDtorRegistration_TourMap[];
extern "C" void PairVector_InitReserved(void*);
extern "C" void MessageDrawPairList_Dtor(); // PointerPairList_Dtor.
extern "C" void PointerPairList_RemoveByKey(void*, int);
extern "C" void* CTourMessage_GetOrCreateMessageDraw(void*);
extern "C" void CMsgDraw_SetDefaultRevealSound(void*, const char*);
extern "C" void CMsgDraw_SetColor(void*, int);
extern "C" void CMsgDraw_ResetTextState(void*);
extern "C" void CMsgDraw_SetRevealStep(void*, int, float);
extern "C" void CMsgDraw_SetGlyphScale(void*, void*);
extern "C" void Text_SetPosition(void*, void*);
extern "C" void Text_SetContent(void*, void*);
extern "C" char tourTipsWindowStrings[];
extern "C" float kFloat_0_5_TourMessage;
extern "C" float kFloat_200_TourMessageSound; // 200: kFloat_200_TourMessageSound.
extern "C" float kFloat_1_TourMessageSound; // 1: kFloat_1_TourMessageSound.
extern "C" int g_soundManager;
extern "C" signed char s_inputManagerInitialized;
extern "C" char s_gameInputManager[];
extern "C" char s_tourMessageInputDtorRegistration[]; // Destructor registration storage for tourMessageInputDtorRegistration.
extern "C" char tourTipsWindowStrings[];
extern "C" void InputManager_Init(void*);
extern "C" void InputManager_Dtor();
extern "C" int Input_IsActionPressed(void*, int, int);
extern "C" int CMsgDraw_IsRevealComplete(void*);
extern "C" int CMsgDraw_NextLine(void*);
extern "C" void CMsgDraw_ClearFormattingAndRevealAll(void*);
extern "C" void CMsgDraw_SetRevealStep(void*, int, float);
extern "C" void Sound_PlayByName(int, const char*, int, int, int, int, float, float, float);
extern "C" char s_bpPlayer[];
extern "C" char tourTipsWindowStrings[];
extern "C" void* kTourMessageGlyphScaleX; // 1.0f glyph X scale; pointer-typed declaration preserves the existing load ABI.
extern void *kTourMessageGlyphScaleY; // 1.0f glyph Y scale; pointer-typed declaration preserves the existing load ABI.
extern "C" void CBpPlayer_Dtor();
extern "C" void CMsgDraw_SetColor(void*, int);
extern "C" void CMsgDraw_ResetTextState(void*);
extern "C" void CMsgDraw_SetRevealStep(void*, int, float);
extern "C" void CMsgDraw_SetGlyphScale(void*, void*);
extern "C" void Text_SetPosition(void*, void*);
extern "C" void Text_SetContent(void*, void*);
extern "C" void CMsgDraw_SetGlyphScale(void*, void*);


// Lazy singleton set-up the original code pasted inline at every use: if the global pointer is still null,
// construct the static object once (guard flag + destructor registration) and publish it in the pointer.
// After the call Sing_805DC670() holds the BpPlayer, Sing_805DC5F0() the BpCtrl, Singleton_TourHintWindow() the tour hint window and
// Sing_805DC650() the message-draw pointer-pair list.
static inline void EnsureBpPlayer() {
    if (!Sing_805DC670()) {
        if (!s_bpPlayerInitialized) {
            CBpPlayer_Ctor(s_bpPlayer);
            __register_global_object(s_bpPlayer, (void*)CBpPlayer_Dtor, s_bpPlayerDtorRegistration_StoryMenu);
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = s_bpPlayer;
    }
}
static inline void EnsureBpCtrl(char* dtorRegistration) {
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl, CBpCtrl_dtor, dtorRegistration); s_bpCtrlInitialized = 1; }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}
static inline void EnsureHintWindow(char* dtorRegistration) {
    if (((int&)Singleton_TourHintWindow()) == 0) {
        if (!s_tourHintWindowInitialized) {
            __ct__15CTourHintWindowFv(s_tourHintWindow);
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

struct TourHintFadeView { char padding00[0x20]; void* items[4]; char padding30[4]; int baseGlyphColor; char padding38[0x14]; int id; int mode; unsigned char opacity; char padding55; unsigned char step; char padding57; int active; };
struct IdTable802FE92C { int values[14]; };
// Advance the hint opacity and apply it to BP groups and four text renderers.
// mode 1 also updates the 14 named BP groups; other modes touch only glyphs.
// With mode == 0 or active == 0, apply baseGlyphColor as-is to existing glyphs;
// hide the BP groups ONLY for mode kGroupsAndGlyphs (1), hence only if inactive.
// Mode zero leaves group opacity untouched and bypasses fade-state advancement.
// Return 1 on that bypass or when entering with fadeState == kFadeComplete;
// an update that reaches the endpoint returns 0 until the next call.
int CTourMessage_UpdateFade(TourHintFadeView* self) {
    int result = 0;
    if (!(self->mode != 0 && self->active)) {
        // Bypass: mode 1 clears BP opacity; every mode applies baseGlyphColor to existing text.
        switch (self->mode) { case kGroupsAndGlyphs: {
            IdTable802FE92C values = *(IdTable802FE92C*)tourInfoBaseGroupNames;
            int* vp = values.values;
            for (unsigned i = 0; i < kHintGroupCount; ++i, ++vp) {
                int vv = *vp;
                EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
                void* obj = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), self->id);
                if (obj) { void* found = BpObject_GetOrCreateNamedGroup(obj, vv); if (found) UIGroup_SetOpacity(found, 0); }
            }
} default: break; case kGlyphsOnly: break;
}
        for (int i = 0; i < kHintTextCount; ++i) if (self->items[i]) CMsgDraw_SetAllGlyphColors((char*)self->items[i], self->baseGlyphColor);
        return 1;
    }
    // Advance one opacity step (per call, not scaled by elapsed time).
    int state = ((CTourMessage*)self)->fadeState;
    switch (state) {
        case kFadeComplete:
            result = 1;
            break;
        case kFadeOut: {
            int delta = (int)self->opacity - (int)self->step;
            // For delta in [-255,255], the arithmetic-shift mask implements max(delta, 0).
            self->opacity = (unsigned char)(delta & (((-delta) & ~delta) >> 31));
            if (!self->opacity) ((CTourMessage*)self)->fadeState = kFadeComplete;
            break;
        }
        case kFadeIn: {
            int value = (int)self->opacity + (int)self->step;
            self->opacity = (unsigned char)(value < kOpaqueAlpha ? value : kOpaqueAlpha);
            if (self->opacity == kOpaqueAlpha) ((CTourMessage*)self)->fadeState = kFadeComplete;
            break;
        }
        default:
            break;
    }
    // Propagate the same alpha to groups and glyphs. Keep each lazy singleton
    // lookup at its use: resource lookup/creation can call into other game code.
    unsigned char amount = self->opacity;
    switch (self->mode) { case kGroupsAndGlyphs: {
        IdTable802FE92C values = *(IdTable802FE92C*)tourInfoBaseGroupNames;
        int* vp = values.values;
        for (unsigned i = 0; i < kHintGroupCount; ++i, ++vp) {
            int vv = *vp;
            EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
            void* obj = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), self->id);
            if (obj) { void* found = BpObject_GetOrCreateNamedGroup(obj, vv); if (found) UIGroup_SetOpacity(found, amount); }
        }
} default: break; case kGlyphsOnly: break;
}
    for (int i = 0; i < kHintTextCount; ++i) if (self->items[i]) CMsgDraw_SetAllGlyphColors((char*)self->items[i], self->baseGlyphColor | amount);
    return result;
}

// Empty hook with no side effects; its interface role remains unresolved.
void fn_802FF430() {}

// Empty hook with no side effects; its interface role remains unresolved.
void fn_802FF6F4() {}

struct TourHintLayoutEntriesView { char padding00[0x48]; int ids[2]; };
// Replace both hint-window BP entries with newly requested resources.
void CTourHintWindow_RecreateLayoutEntries() {
    EnsureHintWindow(s_tourHintWindowDtorRegistration_802FE);
    TourHintLayoutEntriesView* self = (TourHintLayoutEntriesView*)((int&)Singleton_TourHintWindow());
    for (int i = 0; i < 2; ++i) {
        if (self->ids[i] != -1) {
            EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->ids[i]);
            self->ids[i] = -1;
        }
        EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
        CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), tourHintLayoutResourceNames[i], &self->ids[i], 0);
    }
}

struct TourHintOwnedObjectsView { char padding00[0x20]; void* items[4]; char padding30[0x14]; int kind; int ids[2]; };
// Wii pools these separate 12-byte destructor records: BP at +0,
// pointer-pair list at +0xC, input at +0x18, and hint window at +0x24.
enum { kRegPairListOffset = 0xC, kRegHintWindowOffset = 0x24 };
// Release singleton hint text renderers and BP entries, then optionally delete this object.
// In plain C: for each of the four text renderers, remove it from the message-draw pair list and call its
// deleting destructor (virtual slot 2); then delete the two BP layout entries. The text pass is written
// out twice because a shared helper changes the generated code.
void* CTourHintWindow_ReleaseSingletonResources(TourHintOwnedObjectsView* self, int flags) {
    char* regs = (char*)s_tourTipsBpCtrlDtorRegistration;
    if (self) {
    EnsureHintWindow((regs + kRegHintWindowOffset));
    TourHintOwnedObjectsView* state = (TourHintOwnedObjectsView*)((int&)Singleton_TourHintWindow());
    // Pass 1: unregister borrowed registry entries before deleting the owned text objects.
    state->kind = 3;
    for (int i = 0; i < 4; ++i) {
        if (state->items[i]) {
            EnsurePairList((regs + kRegPairListOffset));
            PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)state->items[i]);
        }
        if (state->items[i]) {
            if (state->items[i]) {
                ((void (**)(void*, int))(*(void**)state->items[i]))[2](state->items[i], 1);
            }
            state->items[i] = 0;
        }
    }
    EnsureHintWindow((regs + kRegHintWindowOffset));
    // Pass 2: repeats the original singleton cleanup; normally its item slots are
    // already null. Retain the re-query and pass because destructors may reenter.
    state = (TourHintOwnedObjectsView*)((int&)Singleton_TourHintWindow());
    state->kind = 3;
    for (int i = 0; i < 4; ++i) {
        if (state->items[i]) {
            EnsurePairList((regs + kRegPairListOffset));
            PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)state->items[i]);
        }
        if (state->items[i]) {
            if (state->items[i]) {
                ((void (**)(void*, int))(*(void**)state->items[i]))[2](state->items[i], 1);
            }
            state->items[i] = 0;
        }
    }
    // Release both BP resource IDs after text ownership has been cleared.
    for (int i = 0; i < 2; ++i) {
        if (state->ids[i] != -1) {
            EnsureBpCtrl(regs);
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), state->ids[i]);
            state->ids[i] = -1;
        }
    }
    if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Return true only after both hint-window BP entries finish loading.
bool CTourHintWindow_AreResourcesLoaded() {
    EnsureHintWindow(s_tourHintWindowDtorRegistration_802FE);
    int i = 0;
    int* p = (int*)((int&)Singleton_TourHintWindow());
    for (; i < 2; ++i, ++p) {
        if (p[18] == -1) return 0;
        EnsureBpCtrl(s_tourTipsBpCtrlDtorRegistration);
        if (!BpObject_PollLoad(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), p[18]))) return 0;
    }
    return 1;
}

struct TourSubMessageView { void* vtable; void* ownedMessage; void* items[4]; };
struct SubMessageChild { 
virtual void destroy(int);
};
// Unregister and delete the four child renderers and the owned message object.
void* CTourSubMessage_dtor(TourSubMessageView* self, int flags) {
    if (self) {
        self->vtable=__vt__15CTourSubMessage;
        for(int i=0;i<4;i++) {
            if(self->items[i]) {
                EnsurePairList(s_messageDrawPairListDtorRegistration_TourMap);
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()),(int)self->items[i]);
            }
            if(self->items[i]) { if(self->items[i]) { ((SubMessageChild*)self->items[i])->destroy(1); } self->items[i]=0; }
        }
        if(self->ownedMessage) { if(self->ownedMessage) { ((SubMessageChild*)self->ownedMessage)->destroy(1); } self->ownedMessage=0; }
        if(flags>0) __dl__FPv(self);
    }
    return self;
}

struct TextView { unsigned int flags; unsigned int length; void* longText; };
struct DrawTarget { char padding00[4]; int revealComplete; char padding08[8]; unsigned int color; };
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
void CTourMessage_SetText(DrawTarget* self, TextView* text, void* style) {
    void* params[2] = { kTourMessageGlyphScaleX, kTourMessageGlyphScaleY };
    EnsureBpPlayer();
    void* obj=CTourMessage_GetOrCreateMessageDraw(Sing_805DC670());
    CMsgDraw_SetDefaultRevealSound(obj,tourTipsWindowStrings+0x2e);
    CMsgDraw_SetColor(obj,(self->color&0xffffff00)|0xff);
    CMsgDraw_ResetTextState(obj);
    CMsgDraw_SetRevealStep(obj,1,kFloat_0_5_TourMessage);
    CMsgDraw_SetGlyphScale(obj,params);
    Text_SetPosition(obj,style);
    void* ptr;
    if ((int)text->flags >= 0) {
        ptr = (char*)text + 2;
    } else {
        ptr = text->longText;
    }
    Text_SetContent(obj,ptr);
    self->revealComplete=0;
}

struct TourHintFadeStepView {
	char padding00[0x56];
	unsigned char fadeStep;
};

// Set the opacity change applied on each fade update.
void CTourHintWindow_SetFadeStep(TourHintFadeStepView* self, unsigned char value) {
	self->fadeStep = value;
}

struct TourHintStateView {
	char padding00[0x44];
	int state;
};

// Set the hint window state used by its owner.
void CTourHintWindow_SetState(TourHintStateView* self, int value) {
	self->state = value;
}

struct TourMessageColorView {
	char padding00[0x10];
	int baseGlyphColor;
};

// Set the RGB color mask combined with reveal opacity.
void CTourMessage_SetBaseGlyphColor(TourMessageColorView* self, int value) {
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
void CTourMessage_SetInputMode(TourMessageInputModeView* self, int value) {
	self->inputMode = value;
}

struct TourHintFadeStartView {
	char padding00[0x40];
	int fadeState;
	char state[0x10];
	unsigned char opacity;
};

// Start increasing opacity from fully transparent.
void CTourHintWindow_StartFadeIn(TourHintFadeStartView* self) {
	self->opacity = 0;
	self->fadeState = kFadeIn;
}

// Start decreasing opacity from fully opaque.
void CTourHintWindow_StartFadeOut(TourHintFadeStartView* self) {
	self->opacity = 0xff;
	self->fadeState = kFadeOut;
}

// Test the owner-visible hint-window state.
int CTourHintWindow_IsState(TourHintStateView* self, int value) {
	return self->state == value;
}

struct TourMessageBaseView {
	void* vtable;      // 0x0
	int revealComplete;          // 0x4
	char padding08[0x8];    // 0x8..0xF
	int baseGlyphColor;         // 0x10
	int inputMode;         // 0x14
};

// Initialize the renderer base with white RGB and default input/reveal state.
void CTourMessage_Ctor(TourMessageBaseView* self) {
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
void CTourMessage_ApplyGlyphColorMask(void* self, int value) {
	EnsureBpPlayer();
	void* obj = CTourMessage_GetOrCreateMessageDraw(Sing_805DC670());
	int field = ((TourMessageBaseView*)self)->baseGlyphColor; // 0x802FF260: CTourMessage_Ctor initializes the color at +0x10
	CMsgDraw_SetAllGlyphColors((char*)obj, field | value);
}

struct TourMessageInputView { char padding00[4]; int revealCompleteAtEntry; char padding08[0x0c]; int inputMode; };
// Original 0x802FEEA4: every input query first ensures the shared manager exists.
static inline void* TourMessage_InputManager() {
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_tourMessageInputDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
    }
    return (void*)((int&)Sing_805DC5C8());
}

// Handle reveal skipping and next-line confirmation; return one after the final line.
// The +4 latch records reveal completion observed at entry, even if input changes the renderer.
// allowRevealSkip gates only CMsgDraw_ClearFormattingAndRevealAll while revealing;
// input is still queried when it is zero. Once revealed, confirm advances regardless. Return 1
// only for a confirm on a completed reveal for which NextLine reports no line.
int CTourMessage_ProcessInput(TourMessageInputView* self, int allowRevealSkip) {
    int result = 0;
    EnsureBpPlayer();
    void* obj = CTourMessage_GetOrCreateMessageDraw(Sing_805DC670());
    if (!CMsgDraw_IsRevealComplete(obj)) {
        // Revealing: confirmation may clear formatting; do not advance the line.
        if ((self->inputMode == kConfirmOnlyInput && Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer)) ||
            (self->inputMode == kConfirmOrAlternateInput && (Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer) ||
            Input_IsActionPressed(TourMessage_InputManager(), kAlternateConfirmAction, kAnyPlayer)))) {
            if (allowRevealSkip) CMsgDraw_ClearFormattingAndRevealAll(obj);
        }
        self->revealCompleteAtEntry = 0; // 0x802FEEA4: reveal state at +4
    } else {
        // Revealed: advance once, restart reveal timing, and play the page sound.
        if ((self->inputMode == kConfirmOnlyInput && Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer)) ||
            (self->inputMode == kConfirmOrAlternateInput && (Input_IsActionPressed(TourMessage_InputManager(), kConfirmAction, kAnyPlayer) ||
            Input_IsActionPressed(TourMessage_InputManager(), kAlternateConfirmAction, kAnyPlayer)))) {
            if (!CMsgDraw_NextLine(obj)) result = 1;
            CMsgDraw_SetRevealStep(obj, 0, kFloat_0_5_TourMessage);
            Sound_PlayByName(g_soundManager, tourTipsWindowStrings + kTourMessagePageSound, 0, 0, 0, 0, kFloat_0_5_TourMessage, kFloat_200_TourMessageSound, kFloat_1_TourMessageSound);
    }
        self->revealCompleteAtEntry = 1;
    }
    return result;
}

} // extern "C"
