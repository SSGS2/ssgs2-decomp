#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "game/string.h"
#include "game/eventconte.h"

// Translation unit 0x802FD1D8-0x802FE92C (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CFade.h"

// Borrow the fade layout at this address without changing ownership.
inline CFade* AsCFade(void* object) { return (CFade*)object; }

// CFade is a non-polymorphic alpha ramp; +0 holds its direction/state word.
struct FadeAlphaState { int direction; unsigned char alpha; unsigned char step; };
// View the fade's real scalar layout without changing the generated header.
static inline FadeAlphaState* AsFadeAlpha(void* value) { return (FadeAlphaState*)value; }
enum { kFadeIdle = 0, kFadeDecreaseAlpha = 1, kFadeIncreaseAlpha = 2,
       kAlphaTransparent = 0, kAlphaOpaque = 255, kDefaultFadeStep = 16 };
struct TourHintStrings {
    char nextGroup[7]; char windowGroup[8]; char caddieGroup[8]; char pipinGroup[8];
    char textGroup[15]; char revealSound[9]; char pageSound[15];
};
#define HINT_STRING_OFFSET(member) ((int)&((TourHintStrings*)0)->member)
// Partial view: two script pages, current page selection, and text/callback state.
struct TourHintFields {
    void* vtable; void* nextEnabled; char padding08[8]; int textColor;
    char padding14[0x34]; int pageEntryIds[2]; int currentPage;
    char padding54[0x0C]; int textActive;
};
enum { kHintPageIdsOffset = (int)&((TourHintFields*)0)->pageEntryIds,
       kHintCurrentPageOffset = (int)&((TourHintFields*)0)->currentPage };
// Hint allocation map (byte offsets; shared field names are provisional):
// +00 primary base; +18 submessage base/secondary vtable; +20..2c four owned draw
// objects registered as keys in the shared draw-pair list. Unregister before delete.
// +38 direction/+3c alpha/+3d step form the embedded fade. +40 initializes to 3;
// +44 is set to 3 during draw cleanup (mode meanings unresolved). +48/+4c own
// controller page IDs, with -1 meaning absent. +50 indexes those IDs; +60 textActive.
// Constructor-only bytes +54/+56 and word +58 have no established role here.
enum HintStorageConstants { kHintSubmessageOffset = 0x18, kHintDrawSlotsOffset = 0x20,
    kHintDrawSlotCount = 4, kHintFadeDirectionOffset = 0x38,
    kHintFadeAlphaOffset = 0x3c, kHintFadeStepOffset = 0x3d,
    kHintModeWord40Offset = 0x40, kHintCleanupStateOffset = 0x44,
    kHintPageCount = 2, kInvalidHintPageId = -1, kHintInfoGroupCount = 14,
    kHintCleanupState = 3, kHintInitialWord40 = 3, kHintSecondaryVtableOffset = 0x10 };

// Page IDs: 0 = caddie script/character group, 1 = Pipin script/character group.
// Both have next/window groups; page 1 additionally hides 14 info-base groups.
// currentPage must be 0 or 1 at indexed call sites; SelectPage does not validate.
// nextEnabled is a pointer-sized boolean word passed through the UI ABI.
// Borrow the fields shared by the two script pages and text callbacks.
static inline TourHintFields* AsTourHint(void* value) { return (TourHintFields*)value; }

extern "C" {

extern float kFloat_6_2831855;
float s_twoPi_TourMessages;

int EventScript_RunAndSweep(void*);
void __dl__FPv(void*);

// Attach the result context to the active tour script, then run and sweep it.
int TourScript_RunWithResultContext(void** self, void* value) {
    // value is borrowed result context (+4a0 in TourEventStorage), not owned here.
    // Return normalizes RunAndSweep's nonzero result to 1.
    void* object = *self;
    *reinterpret_cast<void**>(static_cast<unsigned char*>(object) + 0x4A0) = value;
    return EventScript_RunAndSweep(*self) != 0;
}

class D93CObject {
    char padding[0x4A4];
public:
    virtual ~D93CObject();
};

// Delete the owned event object and clear its owner slot.
void Object_DeleteAndClear(D93CObject** self) {
    D93CObject* object = *self;
    if (object != 0) {
        delete object;
        *self = 0;
    }
}

// Free an allocation only when invoked as a deleting destructor.
void* CFade_Dtor(void* ptr, int size) {
    if (ptr != 0 && size > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

// Set the amount of alpha change per update.
void CFade_SetStep(unsigned char* self, unsigned char value) {
    AsCFade(self)->step = value;
}

// Move alpha toward its endpoint; report completion on the following idle update.
int CFade_Update(unsigned char* self) {
    int result = 0;
    switch (AsFadeAlpha(self)->direction) {
    case kFadeIdle:
        result = 1;
        break;
    case kFadeDecreaseAlpha: {
        int value = (int)AsFadeAlpha(self)->alpha - (int)AsFadeAlpha(self)->step;
        value = value > 0 ? value : 0;
        AsFadeAlpha(self)->alpha = (unsigned char)value;
        if ((unsigned char)value == 0) AsFadeAlpha(self)->direction = kFadeIdle;
        break;
    }
    case kFadeIncreaseAlpha: {
        int value = (int)AsFadeAlpha(self)->alpha + (int)AsFadeAlpha(self)->step;
        value = value < kAlphaOpaque ? value : kAlphaOpaque;
        AsFadeAlpha(self)->alpha = (unsigned char)value;
        if ((unsigned char)value == kAlphaOpaque) AsFadeAlpha(self)->direction = kFadeIdle;
        break;
    }
    }
    return result;
}

// Fade the window out by decreasing its opacity from fully opaque.
void CFade_StartFadeOut(unsigned char* self) {
    AsFadeAlpha(self)->alpha = kAlphaOpaque;
    AsFadeAlpha(self)->direction = kFadeDecreaseAlpha;
}

// Initialize a transparent, idle fade with a 16-alpha step.
void CFade_Init(unsigned char* self) {
    AsFadeAlpha(self)->alpha = kAlphaTransparent;
    AsFadeAlpha(self)->step = kDefaultFadeStep;
    AsFadeAlpha(self)->direction = kFadeIdle;
}

// Select the signed tour-hint page index and clear its active-text flag.
void CTourHintWindow_SelectPage(unsigned char* self, signed char value) {
    AsTourHint(self)->currentPage = value;
    AsTourHint(self)->textActive = 0;
}

// Initialize the tour-message angle constant to a full turn in radians.
void TourMessage_InitAngleConstant() {
    s_twoPi_TourMessages = kFloat_6_2831855;
}

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_tourTipsBpCtrlDtorRegistration[];
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void UIGroup_SetEnabled(void*, void*);

extern int tourTipsDefaultScriptNames[2];
extern int tourInfoBaseGroupNames[];
extern char tourTipsWindowStrings[];
int IdMap_Lookup(void*, int);
int BpObject_GetOrCreateNamedGroup(void*, void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void BpObject_SetEnabled(void*, int);
void UIGroup_SetOpacity(void*, void*);
void BpObject_StartScriptByName(void*, int, int);
void BpObject_TryInvokeCurrentCallback(void*, int);

// Lazily publish the shared BP controller; repeated lookups borrow the same singleton.
inline void EnsureHintController() {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_tourTipsBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Hide navigation and character groups on both hint pages, then restart their scripts.
void CTourHintWindow_ResetPageScripts(unsigned char* self) {
    // Keep these declarations in this order for the original register allocation.
    unsigned int j;
    char* base;
    unsigned char* pageIdCursor;
    int* defaultScriptCursor;
    int* next;
    int pageIndex;
    base = tourTipsWindowStrings;
    pageIdCursor = self;
    // Each iteration handles one page: pageIdCursor=self+page*4 makes pageEntryIds[0]
    // address that page's ID. Only that field may use this shifted view.
    // Repeated singleton blocks borrow the same BP controller; they do not
    // allocate a controller per group. Declaration order retains code generation.
    pageIndex = 0;
    defaultScriptCursor = tourTipsDefaultScriptNames;
    do {
        if (AsTourHint(pageIdCursor)->pageEntryIds[0] != kInvalidHintPageId) {
            EnsureHintController();
            void* object = (void*)((int&)Sing_805DC5F0());
            void* item = (void*)IdMap_Lookup(object, AsTourHint(pageIdCursor)->pageEntryIds[0]);
            BpObject_SetEnabled(item, 1);
            EnsureHintController();
            item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
            // Phase 1: make next/window transparent on this page.
            int nextGroup;
            if (item == 0) nextGroup = 0;
            else nextGroup = BpObject_GetOrCreateNamedGroup(item, base);
            if (nextGroup) UIGroup_SetOpacity((void*)nextGroup, 0);
            char* name = base + HINT_STRING_OFFSET(windowGroup);
            EnsureHintController();
            item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
            int windowGroup;
            if (item == 0) windowGroup = 0;
            else windowGroup = BpObject_GetOrCreateNamedGroup(item, name);
            if (windowGroup) UIGroup_SetOpacity((void*)windowGroup, 0);
            // Phase 2: hide the page-specific character and (page 1) info groups.
            switch (pageIndex) {
            case 0: {
                char* name = base + HINT_STRING_OFFSET(caddieGroup);
                EnsureHintController();
                item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(self)->pageEntryIds[0]);
                int characterGroup;
                if (item == 0) characterGroup = 0;
                else characterGroup = BpObject_GetOrCreateNamedGroup(item, name);
                if (characterGroup) UIGroup_SetOpacity((void*)characterGroup, 0);
                break;
            }
            case 1: {
                char* name = base + HINT_STRING_OFFSET(pipinGroup);
                EnsureHintController();
                item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(self)->pageEntryIds[1]);
                int characterGroup;
                if (item == 0) characterGroup = 0;
                else characterGroup = BpObject_GetOrCreateNamedGroup(item, name);
                if (characterGroup) UIGroup_SetOpacity((void*)characterGroup, 0);
                break;
            }
            }
            switch (pageIndex) {
            case 0: break;
            case 1: {
                struct Names { int values[kHintInfoGroupCount]; };
                Names names = *reinterpret_cast<Names*>(tourInfoBaseGroupNames);
                next = names.values;
                j = 0;
                do {
                    void* name = (void*)*next;
                    EnsureHintController();
                    item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(self)->pageEntryIds[1]);
                    int infoGroup;
                    if (item == 0) infoGroup = 0;
                    else infoGroup = BpObject_GetOrCreateNamedGroup(item, name);
                    if (infoGroup) UIGroup_SetOpacity((void*)infoGroup, 0);
                    ++j;
                    ++next;
                } while (j < kHintInfoGroupCount);
                break;
            }
            }
            EnsureHintController();
            item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
            // Phase 3: disable next, restart this page's default script and invoke
            // its initial callback. Finish both pages before clearing textActive.
            int nextGroupToDisable;
            if (item == 0) nextGroupToDisable = 0;
            else nextGroupToDisable = BpObject_GetOrCreateNamedGroup(item, base);
            if (nextGroupToDisable) UIGroup_SetEnabled((void*)nextGroupToDisable, 0);
            EnsureHintController();
            item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
            BpObject_StartScriptByName(item, *defaultScriptCursor, 1);
            EnsureHintController();
            item = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
            BpObject_TryInvokeCurrentCallback(item, 0);
        }
        ++pageIndex;
        pageIdCursor += 4;
        ++defaultScriptCursor;
    } while (pageIndex < kHintPageCount);
    AsTourHint(self)->textActive = 0;
}
#pragma pop
int IdMap_Lookup(void*, int);
void BpObject_Draw(void*);
extern int g_chatMsg;
extern char __vt__12CTourMessage[];
extern char __vt__15CTourSubMessage[];
extern char __vt__15CTourHintWindow[];
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
extern char tourTipsWindowStrings[];
extern char s_bpCtrl[];
extern char s_tourTipsBpCtrlDtorRegistration[];
int BpObject_GetOrCreateNamedGroup(void*, void*);
void UIGroup_GetPosition(int, void*);
void UIGroup_SetOpacity(void*, void*);
void CBpPlayer_Dtor(void);
void CBpPlayer_Ctor(void*);
void* CTourMessage_GetOrCreateMessageDraw(void*);
void CMsgDraw_SetDefaultRevealSound(void*, void*);
void CMsgDraw_SetColor(void*, int);
void CMsgDraw_ResetTextState(void*);
void CMsgDraw_SetRevealStep(void*, int, float);
void CMsgDraw_SetGlyphScale(void*, void*);
void Text_SetPosition(void*, void*);
void Text_SetContent(void*, void*);
extern signed char s_bpPlayerInitialized;
extern int kTourMessageGlyphScaleX;
extern int kTourMessageGlyphScaleY;
extern char s_bpPlayer[];
extern char s_bpPlayerDtorRegistration_StoryMenu[];
extern float kFloat_0_5_TourMessage;
void UIGroup_SetEnabled(void*, void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);

// Position and reveal the current page text with the tour-message sound.
void TourHintMessage_SetText(unsigned char* self, void* arg) {
    // arg borrows the wide-string ABI: heap pointer at +8, inline text at +2.
    // The active page and its text group must already exist. Text is handed to
    // the shared message draw; this wrapper does not take ownership of arg.
    int index = AsTourHint(self)->currentPage;
    void* textGroupName = tourTipsWindowStrings + HINT_STRING_OFFSET(textGroup);
    EnsureHintController();
    int id = AsTourHint(self)->pageEntryIds[index];
    int item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), id);
    int result = 0;
    if (item == 0) {
        result = 0;
    } else {
        result = BpObject_GetOrCreateNamedGroup((void*)item, textGroupName);
    }
    struct { int a; int b; int c; } textPlacement;
    UIGroup_GetPosition(result, &textPlacement.c);
    int messagePlayer = ((volatile int&)Sing_805DC670());
    int defaultTextColor = -256;
    AsTourHint(self)->textColor = defaultTextColor;
    textPlacement.a = kTourMessageGlyphScaleX;
    textPlacement.b = kTourMessageGlyphScaleY;
    if (messagePlayer == 0) {
        if (s_bpPlayerInitialized == 0) {
            CBpPlayer_Ctor(s_bpPlayer);
            __register_global_object(s_bpPlayer, (void*)CBpPlayer_Dtor, s_bpPlayerDtorRegistration_StoryMenu);
            s_bpPlayerInitialized = 1;
        }
        ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
    void* messageDraw = CTourMessage_GetOrCreateMessageDraw((void*)((volatile int&)Sing_805DC670()));
    CMsgDraw_SetDefaultRevealSound(messageDraw, tourTipsWindowStrings + HINT_STRING_OFFSET(revealSound));
    int color = (AsTourHint(self)->textColor & ~255) | 255;
    CMsgDraw_SetColor(messageDraw, color);
    CMsgDraw_ResetTextState(messageDraw);
    CMsgDraw_SetRevealStep(messageDraw, 1, kFloat_0_5_TourMessage);
    CMsgDraw_SetGlyphScale(messageDraw, &textPlacement.a);
    Text_SetPosition(messageDraw, &textPlacement.c);
    void* text;
    // Original 0x802FE4DC: select inline or allocated string storage.
    if ((*reinterpret_cast<unsigned int*>(arg) >> 31) != 0)
    text = *reinterpret_cast<void**>((unsigned char*)arg + 8);
    else
    text = (unsigned char*)arg + 2;
    Text_SetContent(messageDraw, text);
    *reinterpret_cast<int*>(self + 4) = 0;
    AsTourHint(self)->textActive = 1;
}

// Apply opacity to navigation, window and character groups on the current page.
void CTourHintWindow_SetNavigationOpacity(unsigned char* self, void* arg) {
    // arg is an opacity value encoded in the UI's pointer-sized argument word,
    // forwarded unchanged. This unit does not establish whether its units are
    // integer alpha or float bits. Group lookup failure is harmless (skip it).
    // Use field-derived byte offsets here: view helpers alter register lifetimes.
    unsigned char* base = (unsigned char*)tourTipsWindowStrings;
    int index = *reinterpret_cast<int*>(self + kHintCurrentPageOffset);
    EnsureHintController();
    unsigned char* entry = self + index * 4;
    int id = *reinterpret_cast<int*>(entry + kHintPageIdsOffset);
    int found = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), id);
    int group = found != 0 ? BpObject_GetOrCreateNamedGroup((void*)found, base) : 0;
    if (group != 0) UIGroup_SetOpacity((void*)group, arg);
    EnsureHintController();
    found = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), *reinterpret_cast<int*>(entry + kHintPageIdsOffset));
    group = found != 0 ? BpObject_GetOrCreateNamedGroup((void*)found, base + HINT_STRING_OFFSET(windowGroup)) : 0;
    if (group != 0) UIGroup_SetOpacity((void*)group, arg);
    if (index == 0) {
        EnsureHintController();
        found = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), *reinterpret_cast<int*>(self + kHintPageIdsOffset));
        group = found != 0 ? BpObject_GetOrCreateNamedGroup((void*)found, base + HINT_STRING_OFFSET(caddieGroup)) : 0;
        if (group != 0) UIGroup_SetOpacity((void*)group, arg);
    } else if (index == 1) {
        EnsureHintController();
        found = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), *reinterpret_cast<int*>(self + kHintPageIdsOffset + sizeof(int)));
        group = found != 0 ? BpObject_GetOrCreateNamedGroup((void*)found, base + HINT_STRING_OFFSET(pipinGroup)) : 0;
        if (group != 0) UIGroup_SetOpacity((void*)group, arg);
    }
}
void UIGroup_SetEnabled(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void PointerPairList_RemoveByKey(void*, void*);
void CBpCtrl_DeleteEntryById(void*, int);
void CTourSubMessage_dtor(void*, int);
extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];
extern char s_messageDrawPairListDtorRegistration_TourMap[];
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void);

#pragma push
#pragma use_lmw_stmw on
// Unregister and delete hint draw objects, release both script pages and destroy the bases.
// destroy > 0 deletes storage; zero/negative destroys contents only. Return is the original address.
void* CTourHintWindow_dtor(unsigned char* self, int destroy) {
    if (self != 0) {
        *reinterpret_cast<void**>(self) = __vt__15CTourHintWindow;
        *reinterpret_cast<void**>(self + kHintSubmessageOffset) = __vt__15CTourHintWindow + kHintSecondaryVtableOffset;
        *reinterpret_cast<int*>(self + kHintCleanupStateOffset) = kHintCleanupState;
        // Phase 1: remove each registry key before deleting its owned draw object.
        int drawIndex = 0;
        unsigned char* drawSlotCursor = self;
        for (; drawIndex < kHintDrawSlotCount; ++drawIndex, drawSlotCursor += 4) {
            if (*reinterpret_cast<void**>(drawSlotCursor + kHintDrawSlotsOffset) != 0) {
                if (((int&)Sing_805DC650()) == 0) {
                    if (s_pointerPairListInitialized == 0) {
                        PairVector_InitReserved(s_messageDrawPairList);
                        __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_messageDrawPairListDtorRegistration_TourMap);
                        s_pointerPairListInitialized = 1;
                    }
                    ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
                }
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), *reinterpret_cast<void**>(drawSlotCursor + kHintDrawSlotsOffset));
            }
            if (*reinterpret_cast<void**>(drawSlotCursor + kHintDrawSlotsOffset) != 0) {
                if (*reinterpret_cast<void**>(drawSlotCursor + kHintDrawSlotsOffset) != 0) {
                    struct VObj { 
                    virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
                    };
                    VObj* object = *reinterpret_cast<VObj**>(drawSlotCursor + kHintDrawSlotsOffset);
                    object->destroy(1);
                }
                *reinterpret_cast<void**>(drawSlotCursor + kHintDrawSlotsOffset) = 0;
            }
        }
        // Phase 2: controller owns page objects; delete by ID and invalidate each slot.
        unsigned char* pageIdCursor = self;
        int pageIndex = 0;
        for (; pageIndex < kHintPageCount; ++pageIndex, pageIdCursor += 4) {
            if (AsTourHint(pageIdCursor)->pageEntryIds[0] != kInvalidHintPageId) {
                EnsureHintController();
                CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsTourHint(pageIdCursor)->pageEntryIds[0]);
                AsTourHint(pageIdCursor)->pageEntryIds[0] = kInvalidHintPageId;
            }
        }
        // Phase 3: destroy the embedded base, then optionally free the whole allocation.
        CTourSubMessage_dtor(self + kHintSubmessageOffset, 0);
        if (destroy > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

} // extern "C"
struct TourMessageBase {
    unsigned char data[0x18];
    // Initialize the base color, navigation boolean and empty callback state.
    TourMessageBase() {
        *reinterpret_cast<void**>(data) = __vt__12CTourMessage;
        *reinterpret_cast<int*>(data + 0x10) = -256;
        *reinterpret_cast<int*>(data + 4) = 0;
        *reinterpret_cast<int*>(data + 0x14) = 0;
    }
    ~TourMessageBase();
};
struct CTourSubMessage : CEventConte {
    unsigned char data[0x24];
    // Borrow the text factory and create the submessage draw resource.
    CTourSubMessage() {
        unsigned char* submessageBytes = (unsigned char*)this;
        *reinterpret_cast<void**>(submessageBytes) = __vt__15CTourSubMessage;
        void* loaded = *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(g_chatMsg) + 0x10);
        *reinterpret_cast<void**>(submessageBytes + 0x18) = loaded;
        WFixedFont_CreateMsgEcbResource(loaded, submessageBytes + 0x4, 0, 0);
    }
};
struct CTourHintWindow : TourMessageBase, CTourSubMessage {
    unsigned char data[0x24];
    CTourHintWindow();
};
// Initialize both hint-message bases, empty page handles and idle fade/navigation state.
CTourHintWindow::CTourHintWindow() {
    unsigned char* self = (unsigned char*)this;
    unsigned char* submessageBase = self + kHintSubmessageOffset;
    unsigned char* hintBytes = self;
    int zero = 0;
    // Bases are already constructed. Initialize the first fade and publish derived vtables.
    hintBytes[kHintFadeAlphaOffset] = kAlphaTransparent;
    hintBytes[kHintFadeStepOffset] = kDefaultFadeStep;
    *reinterpret_cast<int*>(hintBytes + kHintFadeDirectionOffset) = 0;
    *reinterpret_cast<void**>(hintBytes) = __vt__15CTourHintWindow;
    *reinterpret_cast<void**>(hintBytes + kHintSubmessageOffset) = __vt__15CTourHintWindow + kHintSecondaryVtableOffset;
    *reinterpret_cast<int*>(hintBytes + kHintModeWord40Offset) = kHintInitialWord40;
    // Initialize page/text state. These later stores are not part of the first fade.
    hintBytes[0x54] = 0;
    hintBytes[0x56] = 0x10;
    *reinterpret_cast<int*>(hintBytes + kHintPageIdsOffset) = kInvalidHintPageId;
    *reinterpret_cast<int*>(hintBytes + kHintPageIdsOffset + sizeof(int)) = kInvalidHintPageId;
    *reinterpret_cast<int*>(hintBytes + kHintCurrentPageOffset) = 0;
    *reinterpret_cast<int*>(hintBytes + 0x58) = 0;
    *reinterpret_cast<int*>(hintBytes + kHintCleanupStateOffset) = 0;
    *reinterpret_cast<int*>(hintBytes + 0x60) = 0;
}
extern "C" {

// Refresh the current page navigation state and invoke its script callback.
void TourHint_InvokeCurrentPageCallback(unsigned char* self) {
    unsigned char* hintBytes = self;
    unsigned int index = *reinterpret_cast<unsigned int*>(self + 0x50);
    unsigned char* pageIdCursor = self + (index << 2);
    if (*reinterpret_cast<int*>(pageIdCursor + 0x48) != -1) {
        if (AsTourHint(self)->textActive != 0) {
            char* nextGroupName = tourTipsWindowStrings;
            int id = *reinterpret_cast<int*>(pageIdCursor + 0x48);
            void* nextEnabled = *reinterpret_cast<void**>(self + 4);
            EnsureHintController();
            int item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), id);
            if (item != 0) {
                int found = BpObject_GetOrCreateNamedGroup((void*)item, nextGroupName);
                if (found != 0) {
                    UIGroup_SetEnabled((void*)found, nextEnabled);
                }
            }
        }
        EnsureHintController();
        unsigned int currentPage = *reinterpret_cast<unsigned int*>(hintBytes + 0x50);
        int currentPageId = *reinterpret_cast<int*>(hintBytes + (currentPage << 2) + 0x48);
        int item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), currentPageId);
        BpObject_TryInvokeCurrentCallback((void*)item, 0);
    }
}

// Draw the current hint page if its script entry exists.
void CTourHintWindow_DrawCurrentPage(unsigned char* self) {
    unsigned int index = *reinterpret_cast<unsigned int*>(self + 0x50);
    int id = *reinterpret_cast<int*>(self + 0x48 + index * 4);
    if (id != -1) {
        EnsureHintController();
        int item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), *reinterpret_cast<int*>(self + 0x48 + *reinterpret_cast<unsigned int*>(self + 0x50) * 4));
        BpObject_Draw((void*)item);
    }
}

extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];
extern char s_messageDrawPairListDtorRegistration_TourMap[];
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void);
void PointerPairList_RemoveByKey(void*, void*);

#pragma push
#pragma use_lmw_stmw on
// Unregister and delete all four owned hint draw objects.
void CTourHintWindow_ClearRegisteredObjects(unsigned char* self) {
    unsigned char* cur = self;
    int i = 0;
    *reinterpret_cast<int*>(self + kHintCleanupStateOffset) = kHintCleanupState;
    for (; i < kHintDrawSlotCount; ++i, cur += 4) {
        void** slot = reinterpret_cast<void**>(cur + kHintDrawSlotsOffset);
        if (*slot != 0) {
            if (((int&)Sing_805DC650()) == 0) {
                if (s_pointerPairListInitialized == 0) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_messageDrawPairListDtorRegistration_TourMap);
                    s_pointerPairListInitialized = 1;
                }
                ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), *slot);
        }
        if (*slot != 0) {
            if (*slot != 0) {
                struct VObj { 
                virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
                };
                VObj* object = reinterpret_cast<VObj*>(*slot);
                object->destroy(1);
            }
            *slot = 0;
        }
    }
}
#pragma pop

struct FadeStartState {
    int type;
    unsigned char alpha;
};

// Fade the window in by increasing its opacity from fully transparent.
void CFade_StartFadeIn(FadeStartState* self) {
    self->alpha = kAlphaTransparent;
    self->type = kFadeIncreaseAlpha;
}

void* memset(void*, int, unsigned long);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void appFreeUni__FPv(void*);
void ScriptByteMap_FreeSubtree(void*, void*);
void CEventEffect_Ctor(void*);
extern char __vt__12CEventHandle[];
extern char s_eventHandleEmptyString[];

struct EventByteTreeLinks { EventByteTreeLinks* left; EventByteTreeLinks* right; };
#define WORD(off) (*reinterpret_cast<unsigned int*>(p + (off)))
#define PTR(off) (*reinterpret_cast<void**>(p + (off)))
// Postorder walk: CLEAR1 delegates deeper left/right children then frees its
// node; CLEAR2/3 expand one more level each. CLEAR4 owns the root and resets
// count/root/begin after freeing it. All cursors borrow nodes owned by that tree.
// These expansions deliberately share n1/n2/n3 scratch slots: equivalent inline
// helpers change generated register assignments. CLEAR4B has a distinct scratch
// declaration order for the second tree. Neither path frees the embedded header.
#define CLEAR1(node, off) { n1 = (node); if (n1) { \
    if (n1->left) ScriptByteMap_FreeSubtree(p + (off), n1->left); \
    if (n1->right) ScriptByteMap_FreeSubtree(p + (off), n1->right); \
    appFreeUni__FPv(n1); } }
#define CLEAR2(node, off) { n2 = (node); if (n2) { \
    CLEAR1(n2->left, off); CLEAR1(n2->right, off); appFreeUni__FPv(n2); } }
#define CLEAR3(node, off) { n3 = (node); if (n3) { \
    CLEAR2(n3->left, off); CLEAR2(n3->right, off); appFreeUni__FPv(n3); } }
// Keep the two declaration orders: they preserve each walk's register allocation.
#define CLEAR4(off) { EventByteTreeLinks* n1; EventByteTreeLinks* n2; EventByteTreeLinks* n3; EventByteTreeLinks* n4 = reinterpret_cast<EventByteTreeLinks*>(PTR((off)+4)); if (n4) { \
    CLEAR3(n4->left, off); CLEAR3(n4->right, off); appFreeUni__FPv(n4); \
    WORD(off) = 0; PTR((off)+4) = 0; PTR((off)+8) = reinterpret_cast<void*>(reinterpret_cast<unsigned int>(p) + (off)+4); } }
#define CLEAR4B(off) { EventByteTreeLinks* n4; EventByteTreeLinks* n3; EventByteTreeLinks* n1; EventByteTreeLinks* n2; n4 = reinterpret_cast<EventByteTreeLinks*>(PTR((off)+4)); if (n4) { \
    CLEAR3(n4->left, off); CLEAR3(n4->right, off); appFreeUni__FPv(n4); \
    WORD(off) = 0; PTR((off)+4) = 0; PTR((off)+8) = reinterpret_cast<void*>(reinterpret_cast<unsigned int>(p) + (off)+4); } }


void* ScriptPointerVector_Dtor(void*, int);
void* ScriptObjectTree_FreeNodes(void*, int);
void* CEventEffect_dtor(void*, int);
void* ScriptResourceTree_FreeNodes(void*, int);

#pragma push
#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Swap two bytes while decoding a little-endian table offset.
inline void D1Swap(unsigned char& a, unsigned char& b) { unsigned char temp = a; a = b; b = temp; }
// Event-handle string owner. Retain D1 member type names: their outlined
// destructors are symbol-paired by objdiff and referenced by exception tables.
struct D1String {
    unsigned int words[3];
    // Initialize the event-handle vtable and empty ABI string words.
    D1String(void* vtable, char*& empty, unsigned int& length) {
        empty = s_eventHandleEmptyString;
        length = 0;
        *static_cast<void**>(vtable) = __vt__12CEventHandle;
        words[0] = length;
        words[1] = length;
        words[2] = length;
    }
    // Release owned event-handle string storage using the real string destructor.
    ~D1String() { ((NString*)(this))->~NString(); }
};
// Owned byte-vector member storage.
struct D1Vector {
    unsigned char* begin;
    unsigned int size;
    unsigned int capacity;
    // Initialize an empty owned byte vector.
    D1Vector() { begin = 0; size = 0; capacity = 0; }
    // Release vector storage without deleting the member subobject.
    ~D1Vector() { ScriptPointerVector_Dtor(this, -1); }
    // Remove a byte range from the logical count; no byte destructors are needed.
    void erase(unsigned char* first, unsigned char* last) { size -= last - first; }
    // Empty the byte vector while retaining its allocation.
    void clear() { erase(begin, begin + size); }
};
// Owned primary byte-tree member header.
struct D1Tree {
    unsigned int count;
    EventByteTreeLinks* root;
    EventByteTreeLinks** end;
    // Create an empty byte-tree header with begin pointing to the root slot.
    D1Tree() { count = 0; root = 0; end = &root; }
    // Destroy the primary tree through its matching container specialization.
    ~D1Tree() { ScriptObjectTree_FreeNodes(this, -1); }
};
// Embedded event-effect member storage.
struct D1Other {
    unsigned char bytes[0x14];
    // Initialize the embedded event-effect state.
    D1Other() { CEventEffect_Ctor(this); }
    // Destroy the embedded effect without deleting its member storage.
    ~D1Other() { CEventEffect_dtor(this, -1); }
};
// Owned secondary-tree member header with a distinct destructor.
struct D1Tree2 {
    unsigned int count;
    EventByteTreeLinks* root;
    EventByteTreeLinks** end;
    // Create an empty secondary-tree header with its embedded sentinel.
    D1Tree2() { count = 0; root = 0; end = &root; }
    // Destroy the secondary tree through its distinct specialization.
    ~D1Tree2() { ScriptResourceTree_FreeNodes(this, -1); }
};
struct TourEventStorage {
    // Storage view of the owned event (0x4A8 bytes). data is reset as an 8-byte
    // prefix plus 0x400-byte work area; the event borrows base/resource addresses
    // inside its owner's loaded file buffer. Keep that buffer alive through use.
    // Five byte vectors own their arrays; both tree headers own their nodes.
    // Tree layout is count/root/cached-begin; empty begin == &root. Clearing a
    // tree frees nodes only (byte payload), unlike the banner's owned objects.
    unsigned char data[0x408];
    D1String string;
    void* base;
    void* resource;
    unsigned int fields[6]; // +41c..430: six reset words; consumers/individual roles unresolved.
    // Separate members retain the constructor's individual exception cleanups.
    D1Vector vector0; // +434: owned byte sequence; entry encoding unresolved.
    D1Vector vector1; // +440: distinct owned byte sequence, not aliases of vector0.
    D1Vector vector2; // +44c: owned byte sequence; entry encoding unresolved.
    D1Vector vector3; // +458: owned byte sequence; entry encoding unresolved.
    D1Vector vector4; // +464: owned byte sequence; entry encoding unresolved.
    D1Tree tree;
    D1Other other;
    D1Tree2 tree2;
    unsigned int fields2[2]; // +49c: unresolved word; +4a0: borrowed result context,
                             // set by TourScript_RunWithResultContext after construction.
    void* vtable;
    // Construct the event handle, vectors and trees with separate unwind cleanups.
    TourEventStorage(void* b, void* r, char*& empty, unsigned int& length) : string(&vtable, empty, length) {
#define p reinterpret_cast<unsigned char*>(this)
        if ((string.words[0] >> 31) == 0)
            length = *reinterpret_cast<unsigned char*>(&string) & 0x7F;
        else
            length = string.words[1];
        struct Allocator { unsigned char unused; };
        Allocator allocator;
        Allocator copy = allocator;
        unsigned long len = strlen(empty);
        String_ReplaceRange(&string, 0, length, empty, empty + len, &copy);
        // Phase 1: copy empty event-handle text, bind borrowed resource addresses.
        base = b;
        resource = r;
        memset(data, 0, 8);
        memset(data + 8, 0, 0x400);
        // Phase 2: reset event work/counters, empty five vectors and two trees.
        // Constructors have already registered each member's unwind cleanup.
        fields[0] = 0;
        fields[1] = 0;
        fields[2] = 0;
        fields[3] = 0;
        fields[4] = 0;
        fields[5] = 0;
        vector0.clear();
        vector1.clear();
        vector2.clear();
        vector3.clear();
        vector4.clear();
        CLEAR4(0x470);
        WORD(0x48C) = 0;
        CLEAR4B(0x490);
        fields2[0] = 0;
        fields2[1] = 0;
    }
};
// Do not let the constructor-only receiver macro escape this storage definition.
#undef p
struct TourEventOwnerView { void* object; void** file; };
// Resolve the resource index through the file table, whose offsets are little-endian.
inline void* D1Resource(TourEventOwnerView* self, int index) {
    // Only low eight bits select an entry. No table bounds/load-state checks;
    // caller supplies a loaded file and valid index. Return is borrowed storage.
    unsigned char* table = static_cast<unsigned char*>(CSharedFilePtr_GetLoadedBuffer(*self->file));
    unsigned int temp = reinterpret_cast<unsigned int*>(table)[(unsigned char)index];
    unsigned char* bp = reinterpret_cast<unsigned char*>(&temp);
    D1Swap(bp[0], bp[3]);
    D1Swap(bp[1], bp[2]);
    return table + temp;
}

#pragma push
#pragma use_lmw_stmw on
// Delete the current effect, then construct an event from the selected file-table entry.
void TourEventMessage_ReplaceEffect(TourEventOwnerView* self, int index) {
    // Delete old owned event before allocation. New event borrows the current
    // file buffer; self->file and that buffer must remain valid for its lifetime.
    D93CObject* cur = static_cast<D93CObject*>(self->object);
    if (cur != 0) {
        delete cur;
        self->object = 0;
    }
    char* empty;
    unsigned int length;
    self->object = new TourEventStorage(
        CSharedFilePtr_GetLoadedBuffer(*self->file), D1Resource(self, index), empty, length);
}
#pragma pop
#pragma pop
#undef CLEAR4
#undef CLEAR4B
#undef CLEAR3
#undef CLEAR2
#undef CLEAR1
#undef PTR
#undef WORD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625AC", _ctors_u802FD1D8_0, TourMessage_InitAngleConstant);
#pragma pop
