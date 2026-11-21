#include <decomp/static_init.h>
extern "C" char s_fadeOverlayRegistration_805346CC[];
#include "classes/MWStringStorage.h"
#include "game/chatmsg_match.h"
extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Arm cleanup for a completed string until ownership is explicitly released.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Disarm cleanup after explicit ownership transfer or manual destruction.
    void release() { active = false; }
    // Free heap text only while this unwind cleanup remains armed.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u)) appFreeUni__FPv((void*)words[2]);
    }
};


#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u80335C0C {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Construct an empty narrow string or copy the supplied NUL-terminated text.
    LocalNString_u80335C0C() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Construct an empty narrow string or copy the supplied NUL-terminated text.
    LocalNString_u80335C0C(const char* text) { MWString_ConstructCString(this, text); }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Construct an empty narrow string or copy the supplied NUL-terminated text.
    LocalNString_u80335C0C(const char* text) {
        LocalNString_u80335C0C* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Release narrow-string storage only when the heap tag is set.
    ~LocalNString_u80335C0C() { if (flags >> 31) appFreeUni__FPv(data); }
    // Return the logical character count for inline or heap storage.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow the NUL-terminated narrow text from inline or heap storage.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

// Translation unit 0x80335C0C-0x803371A0 (22 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#define char_array_guard CharArrayGuard_u80335C0C
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "sdk_protos.h"
#include "classes/CPasswordMain.h"
#include "classes/CMsgDraw.h"
#include "classes/WFixedFont.h"
#include <decomp/cw_bitfields.h>

#define AsCPasswordMain(p) ((CPasswordMain*)(p))

// Offsets in the packed password menu string pool; the empty string is CTitleTask's NUL.
enum PasswordMenuString {
    kPasswordPanelPath = 0x00,
    kPasswordBackgroundPath = 0x0D,
    kPasswordCaptionPath = 0x1E,
    kPasswordBackgroundScript = 0x33,
    kPasswordPanelScript = 0x36,
    kPasswordCaptionScript = 0x3F,
    kPasswordReturnTask = 0x45,
    kPasswordEmptyArgument = 0x50,
    kPasswordReturnActor = 0x51,
    kPasswordHelpGroup = 0x5C,
    kPasswordRangeErrorType = 0x6D,
    kPasswordActorClass = 0xA4,
    kPasswordActorName = 0xB2
};
// UI resources: panel/background/caption entry IDs are dynamically allocated, not fixed controls.
// Three labels use s_passwordLabelGroupNames; help uses kPasswordHelpGroup. Nine caption
// groups from s_passwordInitiallyHiddenGroupNames stay hidden. Confirm=4/cancel=6 both leave
// for the title task after the fade permits input; transitionRequested schedules that exit.
enum { kPasswordCount = 3, kPasswordKeyBytes = 8, kPasswordIdentityBytes = 6,
       kPasswordHiddenCaptionGroupCount = 9,
       kPasswordPayloadTag = 0x4D, kPasswordSelectorTag = 0x40,
       kPasswordFadeFrames = 30, kPasswordCancelAction = 6, kPasswordConfirmAction = 4,
       kFadeRegistrationOffset = 12, kInputRegistrationOffset = 24,
       kPasswordUnavailableTextOffset = 0x3A };
// SavePlayerRecord does not yet expose these fields; preserve the record's on-disk offsets.
struct PasswordSaveRecordView {
    char reserved[0x1D8];
    unsigned int unlockedPasswordMask;
    char reservedAfterMask[0x2FC];
    unsigned char passwordIdentity[kPasswordIdentityBytes];
};
struct PasswordTaskInterfaces {
    void* primaryVtable;
    void* processVtable;
    void* taskVtable;
};
typedef char PasswordMaskOffset[((int)&((PasswordSaveRecordView*)0)->unlockedPasswordMask == 0x1D8) ? 1 : -1];
typedef char PasswordIdentityOffset[((int)&((PasswordSaveRecordView*)0)->passwordIdentity == 0x4D8) ? 1 : -1];

extern "C" {

void CTask_StartActorLoading(void*);
void CTask_ProcessActors(void*, float);
void CTask_DisplayActorsByPriority(void*);
void CTask_Ctor(void*);
void CTask_dtor(void*, int);
void __ct__18NativeBase80201D18Fv(void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
int TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void CTask_SetMainActor(void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
extern float kFloat_6_2831855;
extern float s_passwordTaskAngleCache;
extern float s_passwordMainAngleCache;
extern char __vt__13CPasswordTask[];
extern char __vt__13CPasswordMain[];
extern unsigned int g_defaultActorSkipMask;
extern unsigned int g_actorSkipMask;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlRegistration_805346C0[];
void __ct__7CBpCtrlFv(void*);
void* __register_global_object(void*, void*, void*);
void CBpCtrl_dtor();
void InputManager_Dtor();
void BpObject_TryInvokeCurrentCallback(void*, int);
void CMsgDraw_UpdateReveal(void*);
void CTask_RequestTransitionWithFade(void*, const char*, const char*, int);
void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_StartFadeIn(void*, int);
int FadeState_GetState(void*);
void FadeState_RequestFramebufferCapture(void*);
void InputManager_Init(void*);
int Input_IsActionPressed(void*, int, int);
extern void* g_taskManager;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_gameInputManager[];
extern signed char s_inputManagerInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlRegistration_805346C0[];
void __dt__18NativeBase80201D18Fv(void*, int);
void* IdMap_Lookup(void*, int);
void BpObject_Draw(void*);
void Text_DrawGlyphs(void*);
void CBpCtrl_DeleteEntryById(void*, int);
extern char s_passwordTaskTypeDescriptor;
extern char s_passwordMainTypeDescriptor;
void* CPasswordMain_dtor(void*, int);
void* CPasswordTask_dtor(void*, int);
void* SaveData_AcquireInitializedInstance();
void* SaveData_GetCurrentRecord(void*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q33std6detail16char_array_guardFv(void*);

void PasswordData_ClearEightBytes(void*);
void PasswordData_CopyEightBytes(void*, const void*, int);
void Password_EncodeEightBytePayload(void*, const void*, int, void*);
int mbtowc(void*, const void*, int);
void WString_ReplaceRange(void*, int, unsigned int, const void*, const void*, void*);
void SaveData_Ctor(void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char passwordMenuStrings[];
extern char s_passwordWideMessageTemplates[];
extern char kPasswordEncodingKeyByte0;
extern char kPasswordEncodingKeyByte1;
extern char kPasswordEncodingKeyByte2;
extern char kPasswordEncodingKeyByte3;
extern char kPasswordEncodingKeyByte4;
extern char kPasswordEncodingKeyByte5;
extern char kPasswordEncodingKeyByte6;
extern char kPasswordEncodingKeyByte7;
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern char bitsetTestRangeErrorText_804B8178[];
void __dt__Q23std12out_of_rangeFv(void*);
void* WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_GetPosition(void*, void*);
void UIGroup_GetSourceSize(void*, void*);
unsigned int UIGroup_GetTintColor(void*);
void CMsgDraw_SetColor(void*, unsigned int);
void Text_SetPosition(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
void CMsgDraw_SetGlyphScale(void*, void*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Text_SetContent(void*, void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void CMsgDraw_ResetTextState(void*);
extern unsigned int kPasswordGlyphScaleXBits;
extern unsigned int kPasswordGlyphScaleYBits;
struct Words804967D4 { unsigned int firstPassword, secondPassword, thirdPassword; };
extern Words804967D4 s_passwordLabelGroupNames;
extern char s_bpCtrl[];
extern char s_bpCtrlRegistration_805346C0[];
extern void* g_chatMsg;
extern float kFloat_1_PasswordMain;
void CBpCtrl_CreateEntry(void*, const char*, int*, int);
int CBpCtrl_PollAllEntriesLoaded(void*);
void BpObject_StartScriptByName(void*, const char*, int);
void* CMsgDraw_Ctor(void*, void*);
void UIGroup_SetEnabled(void*, int);
void FadeState_StartFadeOut(void*, int);
extern void* g_chatMsg;
extern signed char s_bpCtrlInitialized;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_passwordInitiallyHiddenGroupNames[];

// Forward task actor-loading startup to the base implementation.
void CPasswordTask_StartActorLoading(void* forwarded0) { CTask_StartActorLoading(forwarded0); }
// Forward the frame delta to base actor processing.
void CPasswordTask_ProcessActors(void* forwarded0, float forwarded1) { CTask_ProcessActors(forwarded0, forwarded1); }
// Draw task actors through the base priority-ordered display path.
void CPasswordTask_DisplayActorsByPriority(void* forwarded0) { CTask_DisplayActorsByPriority(forwarded0); }
// Provide the empty password actor preload hook.
void CPasswordMain_OnPreLoadInit() {}
// Provide the empty password actor initialization hook.
void CPasswordMain_OnInit() {}
// Borrow the password task runtime-type descriptor.
void* CPasswordTask_GetRTTI() { return &s_passwordTaskTypeDescriptor; }
// Borrow the password actor runtime-type descriptor.
void* CPasswordMain_GetRTTI() { return &s_passwordMainTypeDescriptor; }
// Adjust the task-main interface pointer back to the owning actor before destruction.
void CPasswordMain_DtorThunk18(void* self, int arg) { CPasswordMain_dtor((char*)self - 0x18, arg); }
// Adjust the task interface pointer back to the owning task before destruction.
void CPasswordTask_DtorThunk8(void* self, int arg) { CPasswordTask_dtor((char*)self - 8, arg); }
// Initialize the password task's cached full-circle angle.
void PasswordTask_InitializeAngleCache() { s_passwordTaskAngleCache = kFloat_6_2831855; }
struct PasswordMessageDrawAllocation {
    char data[0x4bc];
    // Construct the owned message renderer from the borrowed font resource.
    PasswordMessageDrawAllocation(void* a) { CMsgDraw_Ctor(this, a); } };

extern char s_fadeOverlayRegistration_805346CC[];

struct PasswordLoadView {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17();
    char inheritedState[0x18];    // 0x04
    int passwordPanelEntryId;             // 0x1c
    int backgroundEntryId;             // 0x20
    int titleCaptionEntryId;             // 0x24
    void* textRenderer;           // 0x28
    unsigned char transitionRequested;   // 0x2c
};

// Ensure the borrowed BP controller exists; construction and global destructor registration occur once.
static inline void EnsurePasswordBpController() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlRegistration_805346C0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Ensure the borrowed fade singleton exists, registering its owner-supplied destructor record once.
static inline void EnsurePasswordFade(char* registration) {
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, registration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}
// Ensure the borrowed input singleton exists before querying actions; it is globally owned.
static inline void EnsurePasswordInput(char* registration) {
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, registration);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
}

// Load the three UI resources, hide unused groups, build labels and start the entrance fade.
void CPasswordMain_OnLoad(PasswordLoadView* self) {
    EnsurePasswordBpController();
    CBpCtrl_CreateEntry(Sing_805DC5F0(), passwordMenuStrings + kPasswordPanelPath, &self->passwordPanelEntryId, 0);
    EnsurePasswordBpController();
    CBpCtrl_CreateEntry(Sing_805DC5F0(), passwordMenuStrings + kPasswordBackgroundPath, &self->backgroundEntryId, 0);
    EnsurePasswordBpController();
    CBpCtrl_CreateEntry(Sing_805DC5F0(), passwordMenuStrings + kPasswordCaptionPath, &self->titleCaptionEntryId, 0);
    // All three BP requests are issued: allocate the owned text renderer and wait for loading.
    void* mgr = ((CChatMsg*)g_chatMsg)->defaultFixedFont;
    self->textRenderer = new PasswordMessageDrawAllocation(AsWFixedFont(mgr)->messageResource);
    while (1) {
        EnsurePasswordBpController();
        if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0())) break;
    }
    EnsurePasswordBpController();
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->backgroundEntryId), passwordMenuStrings + kPasswordBackgroundScript, 1);
    EnsurePasswordBpController();
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->passwordPanelEntryId), passwordMenuStrings + kPasswordPanelScript, 1);
    // Initialize caption groups only after the panel/background scripts have started.
    unsigned int words[kPasswordHiddenCaptionGroupCount];
    const unsigned int* tbl = (const unsigned int*)s_passwordInitiallyHiddenGroupNames;
    words[0] = tbl[0];
    words[1] = tbl[1];
    words[2] = tbl[2];
    words[3] = tbl[3];
    words[4] = tbl[4];
    words[5] = tbl[5];
    words[6] = tbl[6];
    words[7] = tbl[7];
    words[8] = tbl[8];
    EnsurePasswordBpController();
    void* node = IdMap_Lookup(Sing_805DC5F0(), self->titleCaptionEntryId);
    BpObject_StartScriptByName(node, passwordMenuStrings + kPasswordCaptionScript, 1);
    for (unsigned int i = 0; i < kPasswordHiddenCaptionGroupCount; ++i) {
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(node, (const char*)words[i]), 0);
    }
    // Build labels through the actor hook, then start the 30-frame entrance fade.
    self->v17(); // Virtual slot 17 builds the labels (CPasswordMain_BuildTextLabels).
    EnsurePasswordFade(s_fadeOverlayRegistration_805346CC);
    FadeState_StartFadeOut(Sing_805DC608(), kPasswordFadeFrames);
}
// Update UI callbacks and text, then request the title screen when confirm/cancel is pressed.
void CPasswordMain_OnProcess(void* self) {
    char* dso = s_bpCtrlRegistration_805346C0;
    EnsurePasswordBpController();
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->backgroundEntryId), 0);
    EnsurePasswordBpController();
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->passwordPanelEntryId), 0);
    EnsurePasswordBpController();
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->titleCaptionEntryId), 0);
    CMsgDraw_UpdateReveal(AsCPasswordMain(self)->textRenderer);
    ((CMsgDraw*)AsCPasswordMain(self)->textRenderer)->drawSuppressed = 0;
    // A previous accepted action now issues the title transition and clears the fade immediately.
    if (AsCPasswordMain(self)->transitionRequested) {
        CTask_RequestTransitionWithFade(g_taskManager, passwordMenuStrings + kPasswordReturnTask, passwordMenuStrings + kPasswordEmptyArgument, 0);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, passwordMenuStrings + kPasswordReturnActor, 7, 0, 0, 0, 0);
        EnsurePasswordFade((dso + kFadeRegistrationOffset));
        FadeState_StartFadeIn(Sing_805DC608(), 0);
    }
    EnsurePasswordFade((dso + kFadeRegistrationOffset));
    // Only fade state zero accepts confirm/cancel; latch the exit for the next process tick.
    if (FadeState_GetState(Sing_805DC608()) == 0) {
        EnsurePasswordInput((dso + kInputRegistrationOffset));
        if (!Input_IsActionPressed(Sing_805DC5C8(), kPasswordCancelAction, -1)) {
            EnsurePasswordInput((dso + kInputRegistrationOffset));
            if (!Input_IsActionPressed(Sing_805DC5C8(), kPasswordConfirmAction, -1)) return;
        }
        EnsurePasswordFade((dso + kFadeRegistrationOffset));
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        AsCPasswordMain(self)->transitionRequested = 1;
    }
}
struct PasswordInlineLength {
    CW_BITS2(
        unsigned char isLong : 1,
        unsigned char size : 7)
};

// 12-byte small/heap UTF-16 string; retain the type name used in exception-table relocations.
// Wide-string object is 12 bytes: header + five inline u16 slots, overlaid by
// flags/heap length/heap pointer. Accessors returning references expose storage words,
// not always active values. chars() is borrowed until the next string mutation.
struct WStr36940 {
    unsigned short hdr;
    unsigned short buf[5];
    // Borrow the wide string's tag/capacity word, which overlays inline storage.
    unsigned int& flags() { return *(unsigned int*)this; }
    // Borrow the heap-length word; inline strings encode length in the first tag byte.
    unsigned int& size() { return *((unsigned int*)this + 1); }
    // Borrow the wide string's heap-pointer word, valid as a pointer only for heap strings.
    unsigned short*& data() { return *(unsigned short**)((unsigned int*)this + 2); }
    // Initialize an empty wide string in the packed inline/heap representation.
    WStr36940() { flags() = 0; size() = 0; data() = 0; }
    // Release the wide string's heap buffer when its heap tag is set.
    ~WStr36940() { if ((flags() >> 31) != 0) appFreeUni__FPv(data()); }
    // Make the string empty while preserving its inline/heap allocation mode.
    void clear() {
        if ((flags() >> 31) == 0) {
            buf[0] = 0;
            ((PasswordInlineLength*)this)->size = 0;
        } else {
            *data() = 0;
            size() = 0;
        }
    }
    // Borrow the current NUL-terminated wide-character storage.
    void* chars() {
        if ((flags() >> 31) == 0) return buf;
        return data();
    }
};

struct PasswordTextExpander { 
virtual void m0(int);
};

struct PasswordLabelView {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17();
    virtual void v18(WStr36940*, int);
    char inheritedState[0x18];
    int passwordPanelEntryId;
    int backgroundEntryId;
    int titleCaptionEntryId;
    void* textRenderer;
};

// Move packed tint components into the renderer's channel order and force its low byte to 0xFF.
static inline unsigned int PasswordColorToRgba(unsigned int c) {
    return (c << 24) | ((c << 8) & 0xFF0000) | ((c >> 8) & 0xFF00) | 0xFF;
}

#pragma opt_propagation off
// Expand three generated passwords and the help message into the UI text renderer.
void CPasswordMain_BuildTextLabels(PasswordLabelView* self) {
    const unsigned int* groupNameSource = (const unsigned int*)&s_passwordLabelGroupNames;
    unsigned int labelGroups[3] = { groupNameSource[0], groupNameSource[1], groupNameSource[2] };
    PasswordTextExpander* root;
    void* node;
    float position[2] = { 0, 0 };
    float layoutSize[2] = { 0, 0 };
    union { unsigned int u[2]; float f[2]; } scale;
    scale.u[0] = kPasswordGlyphScaleXBits;
    scale.u[1] = kPasswordGlyphScaleYBits;
    WStr36940 expandedText;
    WStr36940 labelTemplate;
    WFixedFont_CreateMsgEcbResource(((CChatMsg*)g_chatMsg)->defaultFixedFont, &root, 0, 0);
    EnsurePasswordBpController();
    void* bp = IdMap_Lookup(Sing_805DC5F0(), self->passwordPanelEntryId);
    CMsgDraw_ResetTextState(self->textRenderer);
    // Populate each label from its group position, dimensions, tint and generated password.
    for (int i = 0; i < 3; ++i) {
        void* node = BpObject_GetOrCreateNamedGroup(bp, (const char*)labelGroups[i]);
        UIGroup_GetPosition(node, position);
        UIGroup_GetSourceSize(node, layoutSize);
        unsigned int labelColor = PasswordColorToRgba(UIGroup_GetTintColor(node));
        void* renderer = self->textRenderer;
        CMsgDraw_SetColor(renderer, labelColor);
        Text_SetPosition(self->textRenderer, position);
        CMsgDraw_SetLayoutSize(self->textRenderer, layoutSize);
        expandedText.clear();
        labelTemplate.clear();
        self->v18(&labelTemplate, i); // Slot 18 generates password i into caller-owned text.
        Text_ExpandTaggedText(root, &expandedText, (void*)labelTemplate.chars(), 0);
        CMsgDraw_SetGlyphScale(self->textRenderer, scale.f);
        Text_SetContent(self->textRenderer, (void*)expandedText.chars());
    }
    // Append the help message using its own layout and unit glyph scale, then reveal all text.
    node = BpObject_GetOrCreateNamedGroup(bp, passwordMenuStrings + kPasswordHelpGroup);
    unsigned int helpColor = PasswordColorToRgba(UIGroup_GetTintColor(node));
    CMsgDraw_SetColor(self->textRenderer, helpColor);
    UIGroup_GetPosition(node, position);
    UIGroup_GetSourceSize(node, layoutSize);
    Text_SetPosition(self->textRenderer, position);
    CMsgDraw_SetLayoutSize(self->textRenderer, layoutSize);
    scale.f[0] = kFloat_1_PasswordMain;
    scale.f[1] = kFloat_1_PasswordMain;
    CMsgDraw_SetGlyphScale(self->textRenderer, scale.f);
    expandedText.clear();
    Text_ExpandTaggedText(root, &expandedText, s_passwordWideMessageTemplates, 0);
    Text_SetContent(self->textRenderer, (void*)expandedText.chars());
    CMsgDraw_ClearFormattingAndRevealAll(self->textRenderer);
    if (root != 0) {
        if (root != 0) root->m0(1); // Owning destructor dispatch; 1 requests storage release.
        root = 0;
    }
}
#pragma opt_propagation reset
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Encode an unlocked password from its index and six save-identity bytes, or show the locked message.
// outputText is an initialized, caller-owned MW wide string; null is a no-op.
// Caller must provide index 0..2. Other indices merely construct a local out_of_range;
// there is no throw/return in that branch, so it does not validate or reject the index.
// Payload order: 0x4D, (index | 0x40), six save-identity bytes; the eight-byte key
// belongs to the encoder, not the payload. The result replaces the output string.
void CPasswordMain_GetPasswordText(void* selfUnused, void* outputText, unsigned int index) {
    if (outputText == 0) return;
    void* saveRecord = SaveData_GetCurrentRecord(SaveData_AcquireInitializedInstance());
    if (index >= kPasswordCount) {
        std::out_of_range error(bitsetTestRangeErrorText_804B8178, passwordMenuStrings + kPasswordRangeErrorType);
    }
    unsigned int unlockedPasswords = ((PasswordSaveRecordView*)saveRecord)->unlockedPasswordMask;
    if ((unlockedPasswords & (1u << index)) != 0) {
        char encodingKey[kPasswordKeyBytes];
        char passwordEncoder[0x14];
        PasswordData_ClearEightBytes(passwordEncoder);
        encodingKey[0] = kPasswordEncodingKeyByte0;
        encodingKey[1] = kPasswordEncodingKeyByte1;
        encodingKey[2] = kPasswordEncodingKeyByte2;
        encodingKey[3] = kPasswordEncodingKeyByte3;
        encodingKey[4] = kPasswordEncodingKeyByte4;
        encodingKey[5] = kPasswordEncodingKeyByte5;
        encodingKey[6] = kPasswordEncodingKeyByte6;
        encodingKey[7] = kPasswordEncodingKeyByte7;
        PasswordData_CopyEightBytes(passwordEncoder, encodingKey, kPasswordKeyBytes);
        // MW narrow string words: tagged inline storage, or {flags, heapLength, heapData}.
        // Inline characters start at byte 1; bit 31 of word 0 selects heap storage.
        unsigned int encodedText[3] = {};
        EH5StringCleanup eh5Cleanup4(encodedText);
        if (((unsigned int&)Sing_805DC5E0()) == 0) {
            if (s_saveDataInitialized == 0) { SaveData_Ctor(s_saveData); s_saveDataInitialized = 1; }
            ((unsigned int&)Sing_805DC5E0()) = (unsigned int)s_saveData;
        }
        void* currentSaveRecord = SaveData_GetCurrentRecord((void*)((unsigned int&)Sing_805DC5E0()));
        // Build the eight-byte identity payload, then encode into a temporary narrow string.
        char selector[kPasswordKeyBytes] = {};
        selector[0] = kPasswordPayloadTag;
        selector[1] = (char)(index | kPasswordSelectorTag);
        Memory_Copy((void*)(selector + 2), (const void*)((PasswordSaveRecordView*)currentSaveRecord)->passwordIdentity, (size_t)kPasswordIdentityBytes);
        Password_EncodeEightBytePayload(passwordEncoder, selector, kPasswordKeyBytes, encodedText);
        // Convert at most three source bytes per 16-bit character, including NUL.
        // widePassword holds 64 UTF-16 code units (128 bytes). The encoder must fit it:
        // this loop has no destination bound check. mbtowc <= 0 stops the loop;
        // on -1, no terminator is guaranteed before the later wcslen call.
        // Preserve this error behavior; readability changes must not add recovery here.
        char widePassword[0x80];
        unsigned int sourceBytesUsed = 0;
        unsigned int destinationByteOffset = 0;
        int convertedBytes = 0;
        do {
            sourceBytesUsed += convertedBytes;
            const char* src;
            if (((unsigned int)encodedText[0] >> 31) == 0) src = (const char*)encodedText + 1;
            else src = (const char*)encodedText[2];
            void* wideCharacter = widePassword + destinationByteOffset;
            destinationByteOffset += 2;
            convertedBytes = mbtowc(wideCharacter, (const void*)(src + sourceBytesUsed), 3);
        } while (convertedBytes > 0);
        unsigned int length;
        if (*(unsigned int*)outputText >> 31 == 0) length = *(unsigned char*)outputText & 0x7f;
        else length = AsMWStringStorage(outputText)->length;
        // The string ABI takes an address to a stateless allocator byte. Its value
        // is immaterial; the volatile read preserves the original allocator-token setup.
        struct { volatile char a; char pad[3]; } w;
        char ch;
        ch = w.a;
        unsigned int wideLength = wcslen((const wchar_t*)widePassword);
        WString_ReplaceRange(outputText, 0, length, widePassword, widePassword + wideLength * 2, &ch);
        if ((unsigned int)encodedText[0] >> 31) { eh5Cleanup4.release(); appFreeUni__FPv((void*)encodedText[2]); }
    } else {
        char* src = s_passwordWideMessageTemplates + kPasswordUnavailableTextOffset;
        unsigned int length;
        if (*(unsigned int*)outputText >> 31 == 0) length = *(unsigned char*)outputText & 0x7f;
        else length = AsMWStringStorage(outputText)->length;
        // The string ABI takes an address to a stateless allocator byte. Its value
        // is immaterial; the volatile read preserves the original allocator-token setup.
        struct { volatile char a; char pad[3]; } w;
        char ch;
        ch = w.a;
        unsigned int wideLength = wcslen((const wchar_t*)src);
        WString_ReplaceRange(outputText, 0, length, src, src + wideLength * 2, &ch);
    }
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
// Initialize the password actor's cached full-circle angle.
void PasswordMain_InitializeAngleCache() { s_passwordMainAngleCache = kFloat_6_2831855; }
// Destroy the task base and optionally free the password task.
void* CPasswordTask_dtor(void* self, int destroy) {
    if (self != 0) {
        CTask_dtor(self, 0);
        if (destroy > 0) __dl__FPv(self);
    }
    return self;
}
// Initialize the actor interfaces and mark all UI resources as unloaded.
void* CPasswordMain_Ctor(void* self) {
    __ct__18NativeBase80201D18Fv(self);
    *(void**)self = __vt__13CPasswordMain;
    (*(char**)&AsCPasswordMain(self)->taskMainVtable) = __vt__13CPasswordMain + 0x3c;
    AsCPasswordMain(self)->passwordPanelEntryId = -1;
    AsCPasswordMain(self)->backgroundEntryId = -1;
    AsCPasswordMain(self)->titleCaptionEntryId = -1;
    (*(int*)&AsCPasswordMain(self)->textRenderer) = 0;
    (*(char*)&AsCPasswordMain(self)->transitionRequested) = 0;
    return self;
}
typedef LocalNString_u80335C0C LocalString;
// Create the named password actor with an empty argument and make it the main actor.
void CPasswordTask_CreateActors(void* self) {
    char ch;
    struct { volatile char a; char pad[3]; } w;
    LocalString local;
    const char* base = passwordMenuStrings + kPasswordEmptyArgument;
    unsigned int len = strlen(base);
    String_ReserveCapacity(&local, len);
    ch = w.a;
    String_ReplaceRange(&local, 0, 0, base, base + len, &ch);
    int result = TaskActorTable_CreateNamedActor(self, passwordMenuStrings + kPasswordActorClass, passwordMenuStrings + kPasswordActorName, 0, &local);
    CTask_SetMainActor(self, result);
}
// Initialize the task interfaces and clear both actor-skip masks.
void* CPasswordTask_Ctor(void* self) {
    CTask_Ctor(self);
    *(void**)self = &__vt__13CPasswordTask;
    ((PasswordTaskInterfaces*)self)->processVtable = (char*)&__vt__13CPasswordTask + 0x10;
    ((PasswordTaskInterfaces*)self)->taskVtable = (char*)&__vt__13CPasswordTask + 0x1c;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
    return self;
}

// Draw the background, password panel, caption and generated text in order.
void CPasswordMain_OnDisplay(void* self) {
    EnsurePasswordBpController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->backgroundEntryId));
    EnsurePasswordBpController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->passwordPanelEntryId));
    EnsurePasswordBpController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCPasswordMain(self)->titleCaptionEntryId));
    Text_DrawGlyphs(AsCPasswordMain(self)->textRenderer);
}

struct PasswordRendererOwner { 
virtual void m1(int);
};
// Delete the text renderer and release each loaded UI entry.
void CPasswordMain_OnDestroy(void* self) {
    if (AsCPasswordMain(self)->textRenderer != 0) {
        if (AsCPasswordMain(self)->textRenderer != 0) {
            PasswordRendererOwner* obj = (PasswordRendererOwner*)AsCPasswordMain(self)->textRenderer;
            obj->m1(1); // Destroy the owned renderer and release its storage.
        }
        AsCPasswordMain(self)->textRenderer = 0;
    }
    if (AsCPasswordMain(self)->passwordPanelEntryId != -1) {
        EnsurePasswordBpController();
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->passwordPanelEntryId);
        AsCPasswordMain(self)->passwordPanelEntryId = -1;
    }
    if (AsCPasswordMain(self)->backgroundEntryId != -1) {
        EnsurePasswordBpController();
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->backgroundEntryId);
        AsCPasswordMain(self)->backgroundEntryId = -1;
    }
    if (AsCPasswordMain(self)->titleCaptionEntryId != -1) {
        EnsurePasswordBpController();
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->titleCaptionEntryId);
        AsCPasswordMain(self)->titleCaptionEntryId = -1;
    }
}

// Restore the actor interfaces, release UI ownership and optionally free the actor.
void* CPasswordMain_dtor(void* self, int destroy) {
    if (self != 0) {
        PasswordRendererOwner* obj = (PasswordRendererOwner*)AsCPasswordMain(self)->textRenderer;
        *(void**)self = __vt__13CPasswordMain;
        (*(char**)&AsCPasswordMain(self)->taskMainVtable) = __vt__13CPasswordMain + 0x3c;
        if (obj != 0) {
            if (obj != 0) {
                obj->m1(1); // Destroy the owned renderer and release its storage.
            }
            AsCPasswordMain(self)->textRenderer = 0;
        }
        if (AsCPasswordMain(self)->passwordPanelEntryId != -1) {
            EnsurePasswordBpController();
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->passwordPanelEntryId);
            AsCPasswordMain(self)->passwordPanelEntryId = -1;
        }
        if (AsCPasswordMain(self)->backgroundEntryId != -1) {
            EnsurePasswordBpController();
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->backgroundEntryId);
            AsCPasswordMain(self)->backgroundEntryId = -1;
        }
        if (AsCPasswordMain(self)->titleCaptionEntryId != -1) {
            EnsurePasswordBpController();
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPasswordMain(self)->titleCaptionEntryId);
            AsCPasswordMain(self)->titleCaptionEntryId = -1;
        }
        if (self != 0) {
            __dt__18NativeBase80201D18Fv(self, 0);
        }
        if (destroy > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462644", _ctors_u80335C0C_0, PasswordTask_InitializeAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU80462644", _ctors_u80335C0C_1, PasswordMain_InitializeAngleCache);
#pragma pop
