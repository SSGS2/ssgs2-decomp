// The story event commands that carry message text: the request command expands
// a localized message into an event-owned WString, and the window command opens,
// closes or advances the shared message window. Both use the CBpPlayer message
// renderer and speaker state.
#include <decomp/static_init.h>
typedef unsigned long new_size_t;
extern "C" char s_bpCtrlDtorRegistration_StoryMessage[];
extern "C" char s_bpPlayerDestructorRegistration[];
extern "C" char s_inputManagerDtorRegistration_StoryEvent[];
extern "C" char s_partyItemVectorDtorRegistration[];

#include "game/singletons.h"
#include "game/wstring.h"
#include "classes/CEventMessageSC.h"
#include "classes/CBpPlayer.h"
#include "classes/CMsgDraw.h"
#include "classes/CBpCtrl.h"

// View borrowed storage through the recovered story-player layout.
static inline CBpPlayer* AsCBpPlayer(void* player) {
    return (CBpPlayer*)player;
}
// Preserve integer-address callers used by singleton accessors.
static inline CBpPlayer* AsCBpPlayer(int player) {
    return (CBpPlayer*)player;
}
// CBpPlayer singleton fields: control entry +0x10, loaded data +0xC,
// message objects +0x38 and tagged script strings +0x40..+0x78.

// View command-specific event storage without allocating or changing ownership.
static inline CEventMessageSC* AsCEventMessageSC(void* event) {
    return (CEventMessageSC*)event;
}
struct CEventConte {
    void* vtable;
    // Leave base storage for the derived constructor to initialize.
    CEventConte() {}
    // Install the supplied event-command interface.
    CEventConte(void* table) : vtable(table) {}
};
extern "C" void* __nw__FUl(unsigned long);
extern "C" void __dl__FPv(void*) __attribute__((nothrow));
// Allocate event/renderer storage through the game allocator.
inline void* operator new(new_size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

enum {
    kStoryInputInitial = 0,
    kStoryInputArmed = 1,
    kStoryInputPressed = 2,
    kStoryAdvanceAction = 4,
    kAllStoryPlayers = -1,
    kStoryAdvanceRequired = 60,
    kStoryAlphaOpaque = 255,
    kStoryWindowClose = 0,
    kStoryWindowOpen = 1,
    kStoryWindowAdvance = 2,
    kStoryNoSpeaker = 5,
    kStoryMissingControlEntry = -1
};
struct StoryMessageStringPool {
    char resourceNames[0xA2];
    char revealSound[19];
    char messageGroup[12];
    char pageSound[15];
};
#define STORY_STRING_OFFSET(member) ((int)&((StoryMessageStringPool*)0)->member)
struct StoryLoadedFontView {
    char padding00[0xAC];
    void* fontResource;
};

extern "C" {

extern float s_twoPi_800A2158;
extern const float kFloat_6_2831855;
extern void appFreeUni__FPv(void*);
extern signed char s_bpPlayerInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_courseSlotTableInitialized;
extern char s_bpPlayer[];
char s_inputManagerDtorRegistration_StoryEvent[12];
char s_partyItemVectorDtorRegistration[12];
char s_bpPlayerDestructorRegistration[12];
char s_bpCtrlDtorRegistration_StoryMessage[12];
extern char s_bpCtrl[];
extern char s_messageDrawPairList[];
extern char __vt__23CEventMessageSC_MSG_WIN[];
extern char __vt__23CEventMessageSC_MSG_REQ[];
extern char s_conteModelSlots[];
extern const char storyMessageStrings[];
extern char s_gameInputManager[];
extern void __dt__9CBpPlayerFv();
extern void __register_global_object(void*, void (*)(), void*) __attribute__((nothrow));
extern void PairVector_InitReserved(void*);
extern void MessageDrawPairList_Dtor();
extern void PointerPairList_RemoveByKey(void*, void*);
extern void BpObject_StartScriptByName(int, void*, int);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern int IdMap_Lookup(void*, int);
extern void CBpCtrl_dtor();
extern void __register_global_object(void*, void (*)(), void*) __attribute__((nothrow));
extern void CBpCtrl_dtor();
extern int IdMap_Lookup(void*, int);
extern int BpObject_IsCurrentScriptComplete(int);
extern void StoryMessage_SelectTransitionScripts(void*, int);
extern void EventMessage_RebuildMessageDraw(void*);
extern void StoryMessage_SelectTransitionScripts(void*, int);
extern int SlotTable_GetEntry(void*, int);
extern unsigned char CourseSlotTable_IsExtraSlotRange(void*, int);
extern void CourseSlotTable_InitEmpty(void*);
extern void PointerIntVector_AppendPair(void*, void*, int);
extern void Text_SetPosition(void*, void*);
extern void Text_SetContent(void*, void*);
extern int BpObject_GetOrCreateNamedGroup(int, const char*);
extern void UIGroup_GetPosition(int, void*);
extern int Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern signed char s_inputManagerInitialized;
extern int g_soundManager;
extern const float kFloat_0_5_StoryMessage;
extern const float kFloat_5_StoryMessage;
extern const float kFloat_200_StoryMessage;
extern const float kFloat_1_StoryMessage;
extern const unsigned int kFloat_1_StoryGlyphScale;
extern const unsigned int kFloatBits_1_StoryMessageGlyphScaleY;

struct StoryMessageDrawAllocation {
    char pad[0x4BC];
    // Construct a retained text renderer using the borrowed loaded font resource.
    StoryMessageDrawAllocation(void* p) {
        ((CMsgDraw*)this)->__ct(p);
    }
};

// Borrow the script controller's ID map; this is an object address, while
// CBpPlayer::controlEntryId is a key inside it. Never cache across script calls.
static inline void* StoryControlMap() {
    return (void*)((int&)Sing_805DC5F0());
}

// Constructors use offsets in the packed registration block; polling uses the
// corresponding standalone symbols. Keep both forms to preserve relocation targets.
// Construct/register once and publish the singleton pointer; registrationBase is explicit.
inline void EnsureStoryPlayer(char* registrationBase) {
    if (((int&)Sing_805DC670()) == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (registrationBase + 0x18));
            s_bpPlayerInitialized = 1;
        }
        ((int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
}
// Construct/register once and publish the singleton pointer; registrationBase is explicit.
inline void EnsureMessageDrawRegistry(char* registrationBase) {
    if (((int&)Sing_805DC650()) == 0) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor, (registrationBase + 0xC));
            s_pointerPairListInitialized = 1;
        }
        ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
}
// Construct/register once and publish the singleton pointer; registrationBase is explicit.
inline void EnsureBpControl(char* registrationBase) {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, (registrationBase + 0x24));
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

// Lazily construct/register the story player at this access point.
static inline void EnsureRequestStoryPlayer() {
    if (((int&)Sing_805DC670()) == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDestructorRegistration);
            s_bpPlayerInitialized = 1;
        }
        ((int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
}

// Lazily construct/register the script control map at this access point.
static inline void EnsureRequestStoryControl() {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

// Lazily construct/register the input manager at this access point.
static inline void EnsureRequestStoryInput() {
    if (((int&)GetInputManagerSlot()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_StoryEvent);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
}

// Lazily construct/register the renderer ownership registry at this access point.
static inline void EnsureRequestDrawRegistry() {
    if (((int&)Sing_805DC650()) == 0) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                     s_partyItemVectorDtorRegistration);
            s_pointerPairListInitialized = 1;
        }
        ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
}

// Access the four-byte MSG_REQ latch; MSG_WIN uses only the byte at +4.
static inline int& RequestReadyScriptStarted(void* request) {
    return AsCEventMessageSC(request)->unk_4;
}

// Resolve the borrowed player's active control object, or zero for entry ID -1.
// Re-resolve at each call: script changes and lazy construction remain observable.
static inline int ResolveRequestControlObject(void* storyPlayer) {
    int controlObjectWord;
    if (AsCBpPlayer(storyPlayer)->controlEntryId != kStoryMissingControlEntry) {
        EnsureRequestStoryControl();
        controlObjectWord = IdMap_Lookup(StoryControlMap(), AsCBpPlayer(storyPlayer)->controlEntryId);
    } else {
        controlObjectWord = 0;
    }
    return controlObjectWord;
}

// Request lifecycle (one poll can fall through several rows):
//   constructor: choose speaker -> start request script -> own expanded text;
//   poll: script done -> prepare renderer once -> reveal current line;
//   reveal done: start ready script once -> wait for its completion next poll;
//   ready done + advance: NextLine -> start advance script -> clear ready/script latches;
//   no next line: return 1; caller destroys request text, renderer remains registered.
// A press during reveal clears formatting/reveal trees; a press after readiness
// advances only when input is enabled or the shared advanceRequested flag is set.
// +4 is a REQUEST word latch, a WINDOW byte command, and a PARTY payload word:
// the shared CEventMessageSC view does not establish one common semantic field.
// Wait for scripts, prepare/reveal text and advance lines only when the required input arrives.
// Returns 1 only when NextLine reports no more lines; 0 means this request is pending.
// Latches: scriptCompletionLatched gates current-script polling; messageDrawPrepared
// gates initial text setup; RequestReadyScriptStarted names MSG_REQ's +4 word.
// advanceInputState arms on the first update, then records reveal/advance input.
// advanceInputRequired is used as a boolean here (60 is not decremented here).
// Singleton lookups may construct/register objects; preserve their access order.
int CEventMessageSC::MSG_REQ_UpdateMessage() {
    unsigned char* s = (unsigned char*)this;
    void* storyPlayer;
    // Phase 1: arm input without accepting a press on the initial update.
    if (AsCEventMessageSC(s)->advanceInputState == kStoryInputInitial) {
        AsCEventMessageSC(s)->advanceInputState = kStoryInputArmed;
    } else if (AsCEventMessageSC(s)->inputDisabled == 0) {
        EnsureRequestStoryInput();
        if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kStoryAdvanceAction, kAllStoryPlayers) != 0)
            AsCEventMessageSC(s)->advanceInputState = kStoryInputPressed;
    }
    // Phase 2: wait for the request/ready/advance script, then latch completion.
    if (AsCEventMessageSC(s)->scriptCompletionLatched == 0) {
        EnsureRequestStoryPlayer();
        storyPlayer = (void*)((int&)Sing_805DC670());
        int controlObjectWord = ResolveRequestControlObject(storyPlayer);
        if (BpObject_IsCurrentScriptComplete(controlObjectWord) == 0)
            return 0;
        AsCEventMessageSC(s)->scriptCompletionLatched = 1;
    }
    // Phase 3: create/register the retained renderer and install text once.
    if (AsCEventMessageSC(s)->messageDrawPrepared == 0) {
        EnsureRequestStoryPlayer();
        storyPlayer = (void*)((int&)Sing_805DC670());
        int controlObjectWord = ResolveRequestControlObject(storyPlayer);
        struct MessagePosition {
            float x;
            float y;
        } messagePosition;
        UIGroup_GetPosition(
            BpObject_GetOrCreateNamedGroup(controlObjectWord, storyMessageStrings + STORY_STRING_OFFSET(messageGroup)),
            &messagePosition);
        messagePosition.y += kFloat_5_StoryMessage;
        EnsureRequestStoryPlayer();
        storyPlayer = (void*)((int&)Sing_805DC670());
        if ((*(int*)&AsCBpPlayer(storyPlayer)->messageDraw) == 0) {
            StoryMessageDrawAllocation* made = new StoryMessageDrawAllocation(
                ((StoryLoadedFontView*)AsCBpPlayer(storyPlayer)->fixedFontRenderer)->fontResource);
            EnsureRequestDrawRegistry();
            PointerIntVector_AppendPair((void*)((int&)Sing_805DC650()), made, 1);
            AsCBpPlayer(storyPlayer)->messageDraw = made;
            ((CMsgDraw*)made)->SetDefaultRevealSound(storyMessageStrings + STORY_STRING_OFFSET(revealSound));
        }
        void* renderer = AsCBpPlayer(storyPlayer)->messageDraw;
        ((CMsgDraw*)renderer)->ResetTextState();
        ((CMsgDraw*)renderer)->SetRevealStep(1, kFloat_0_5_StoryMessage);
        unsigned int glyphScaleBits[2] = {kFloat_1_StoryGlyphScale, kFloatBits_1_StoryMessageGlyphScaleY};
        ((CMsgDraw*)renderer)->SetGlyphScale((float*)glyphScaleBits);
        Text_SetPosition(renderer, &messagePosition);
        void* messageText;
        if ((AsCEventMessageSC(s)->messageStorage >> 31) == 0)
            messageText = s + 0x16;
        else
            messageText = AsCEventMessageSC(s)->messageHeapData;
        // Text storage is borrowed from the request for this call; the event
        // retains its inline/heap expanded string until request destruction.
        Text_SetContent(renderer, messageText);
        AsCEventMessageSC(s)->advanceInputRequired = kStoryAdvanceRequired;
        EnsureRequestStoryPlayer();
        storyPlayer = (void*)((int&)Sing_805DC670());
        AsCBpPlayer(storyPlayer)->messageAlpha = kStoryAlphaOpaque;
        EventMessage_RebuildMessageDraw(storyPlayer);
        AsCEventMessageSC(s)->messageDrawPrepared = 1;
    }
    EnsureRequestStoryPlayer();
    storyPlayer = (void*)((int&)Sing_805DC670());
    if ((*(int*)&AsCBpPlayer(storyPlayer)->messageDraw) == 0) {
        StoryMessageDrawAllocation* made = new StoryMessageDrawAllocation(
            ((StoryLoadedFontView*)AsCBpPlayer(storyPlayer)->fixedFontRenderer)->fontResource);
        EnsureRequestDrawRegistry();
        PointerIntVector_AppendPair((void*)((int&)Sing_805DC650()), made, 1);
        AsCBpPlayer(storyPlayer)->messageDraw = made;
        ((CMsgDraw*)made)->SetDefaultRevealSound(storyMessageStrings + STORY_STRING_OFFSET(revealSound));
    }
    void* renderer = AsCBpPlayer(storyPlayer)->messageDraw;
    // Phase 4: reveal -> start ready script -> wait for advance -> next line.
    // A new line clears the ready/script latches but retains prepared text.
    // 0x800A26A0: Stop message advancement while reveal or input is pending.
    do {
        if (((CMsgDraw*)renderer)->IsRevealComplete() == 0) {
            if (AsCEventMessageSC(s)->advanceInputState == kStoryInputPressed)
                ((CMsgDraw*)renderer)->ClearFormattingAndRevealAll();
            break;
        }
        if (RequestReadyScriptStarted(s) == 0) {
            EnsureRequestStoryPlayer();
            char* obj = (char*)((int&)Sing_805DC670());
            void* scriptName;
            if ((AsCBpPlayer(obj)->messageReadyScriptFlags >> 31) == 0)
                scriptName = AsCBpPlayer(obj)->stringBytes_64 + 1;
            else
                scriptName = AsCBpPlayer(obj)->messageReadyScriptData;
            int controlObjectWord = ResolveRequestControlObject(obj);
            BpObject_StartScriptByName(controlObjectWord, scriptName, 1);
            RequestReadyScriptStarted(s) = 1;
            AsCEventMessageSC(s)->scriptCompletionLatched = 0;
        } else {
            if (AsCEventMessageSC(s)->advanceInputRequired != 0) {
                EnsureRequestStoryPlayer();
                if ((*(int*)&AsCBpPlayer((int&)Sing_805DC670())->advanceRequested) == 0) {
                    if (AsCEventMessageSC(s)->inputDisabled != 0)
                        break;
                    EnsureRequestStoryInput();
                    if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kStoryAdvanceAction,
                                              kAllStoryPlayers) == 0)
                        break;
                }
            }
            if (((CMsgDraw*)renderer)->NextLine() == 0) {
                ((CMsgDraw*)renderer)->SetRevealStep(0, kFloat_0_5_StoryMessage);
                Sound_PlayByName((void*)g_soundManager, storyMessageStrings + STORY_STRING_OFFSET(pageSound), 0, 0, 0,
                                 0, kFloat_0_5_StoryMessage, kFloat_200_StoryMessage, kFloat_1_StoryMessage);
                return 1;
            }
            AsCEventMessageSC(s)->advanceInputState = kStoryInputInitial;
            AsCEventMessageSC(s)->advanceInputRequired = kStoryAdvanceRequired;
            ((CMsgDraw*)renderer)->SetRevealStep(0, kFloat_0_5_StoryMessage);
            Sound_PlayByName((void*)g_soundManager, storyMessageStrings + STORY_STRING_OFFSET(pageSound), 0, 0, 0, 0,
                             kFloat_0_5_StoryMessage, kFloat_200_StoryMessage, kFloat_1_StoryMessage);
            EnsureRequestStoryPlayer();
            char* obj = (char*)((int&)Sing_805DC670());
            void* scriptName;
            if ((AsCBpPlayer(obj)->messageAdvanceScriptFlags >> 31) == 0)
                scriptName = AsCBpPlayer(obj)->stringBytes_70 + 1;
            else
                scriptName = AsCBpPlayer(obj)->messageAdvanceScriptData;
            int controlObjectWord = ResolveRequestControlObject(obj);
            BpObject_StartScriptByName(controlObjectWord, scriptName, 1);
            EnsureRequestStoryPlayer();
            storyPlayer = (void*)((int&)Sing_805DC670());
            AsCBpPlayer(storyPlayer)->advanceRequested = 0;
            RequestReadyScriptStarted(s) = 0;
            AsCEventMessageSC(s)->scriptCompletionLatched = 0;
        }
    } while (0);
    return 0;
}

extern void MessageTable_ExpandEntryText(void*, void**, unsigned short, int);
extern void* BpObject_SetEnabled(void*, int);
extern void* __nw__FUl(unsigned long);
extern void PointerIntVector_AppendPair(void*, void*, int);
extern char __vt__Q25party5IItem[];

#pragma push
#pragma use_lmw_stmw on
} // extern "C"
// Native Wii command layout. The producer supplies at least five bytes, with
// a 2-byte-aligned u16 ID in Wii big-endian byte order; this view borrows those bytes.
struct StoryTextRequestPayload {
    unsigned char opcode;
    unsigned char speakerSlot; // 5 hides the speaker; other values select a slot
    unsigned short localizedMessageId;
    unsigned char disableUserInput;
};
typedef char StoryTextRequest_idOffset[((int)&((StoryTextRequestPayload*)0)->localizedMessageId == 2) ? 1 : -1];
// Borrow the packed request schema without retaining the caller's command buffer.
static inline const StoryTextRequestPayload& TextRequest(const unsigned char* bytes) {
    return *(const StoryTextRequestPayload*)bytes;
}

struct CEventMessageSC_MSG_REQ : CEventConte {
    char fields[0x10];
    WString text;
    char flags[2];
    CEventMessageSC_MSG_REQ(unsigned char* arg);
    ~CEventMessageSC_MSG_REQ();
};
// Resolve the speaker, start the message script and expand the requested localized text.
// Borrowed command bytes: [0] opcode, [1] speaker slot (5=no speaker),
// [2..3] native-endian u16 localized message ID, [4] disable user input.
// Expanded text belongs to this event; the shared renderer is registered separately.
CEventMessageSC_MSG_REQ::CEventMessageSC_MSG_REQ(unsigned char* arg) : CEventConte(__vt__23CEventMessageSC_MSG_REQ) {
    void* self = this;
    unsigned int requestControlObjectWord;
    unsigned int enabledControlObjectWord;
    char* const base = s_inputManagerDtorRegistration_StoryEvent;
    // Phase 1: choose speaker/transition scripts, then rebuild the shared panel.
    if (kStoryNoSpeaker == TextRequest(arg).speakerSlot) {
        EnsureStoryPlayer(base);
        void* obj = (void*)((int&)Sing_805DC670());
        StoryMessage_SelectTransitionScripts(obj, -1);
        AsCBpPlayer(obj)->speakerIndex = -1;
        AsCBpPlayer(obj)->speakerTableIndex = -1;
        AsCBpPlayer(obj)->speakerSlotIndex = -1;
        EventMessage_RebuildMessageDraw(obj);
    } else {
        EnsureStoryPlayer(base);
        int id;
        unsigned char* entry;
        void* obj;
        obj = (void*)((int&)Sing_805DC670());
        id = TextRequest(arg).speakerSlot;
        StoryMessage_SelectTransitionScripts(obj, id);
        if (id < 0) {
            AsCBpPlayer(obj)->speakerIndex = -1;
            AsCBpPlayer(obj)->speakerTableIndex = -1;
            AsCBpPlayer(obj)->speakerSlotIndex = -1;
        } else {
            if (0 == ((int&)GetConteModelSlotsSlot())) {
                if (!s_courseSlotTableInitialized) {
                    CourseSlotTable_InitEmpty(s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
            }
            void* lookup = (void*)((int&)GetConteModelSlotsSlot());
            entry = (unsigned char*)SlotTable_GetEntry(lookup, id);
            AsCBpPlayer(obj)->speakerSlotIndex = id;
            AsCBpPlayer(obj)->speakerTableIndex = CourseSlotTable_IsExtraSlotRange(lookup, id);
            AsCBpPlayer(obj)->speakerIndex = entry[0];
        }
        EventMessage_RebuildMessageDraw(obj);
    }
    EnsureStoryPlayer(base);
    void* obj = (void*)((int&)Sing_805DC670());
    // Phase 2: start/enable the request script, then copy expanded localized text into the event.
    unsigned char* requestScriptName;
    if ((AsCBpPlayer(obj)->messageRequestScriptFlags >> 31) == 0)
        requestScriptName = (AsCBpPlayer(obj)->stringBytes_40 + 1);
    else
        requestScriptName = (unsigned char*)AsCBpPlayer(obj)->messageRequestScriptData;

    if (AsCBpPlayer(obj)->controlEntryId != kStoryMissingControlEntry) {
        EnsureBpControl(base);
        requestControlObjectWord = IdMap_Lookup(StoryControlMap(), AsCBpPlayer(obj)->controlEntryId);
    } else {
        requestControlObjectWord = 0;
    }
    BpObject_StartScriptByName(requestControlObjectWord, requestScriptName, 1);
    EnsureStoryPlayer(base);
    void* playerToEnable = (void*)((int&)Sing_805DC670());

    if (AsCBpPlayer(playerToEnable)->controlEntryId != kStoryMissingControlEntry) {
        EnsureBpControl(base);
        enabledControlObjectWord = IdMap_Lookup(StoryControlMap(), AsCBpPlayer(playerToEnable)->controlEntryId);
    } else {
        enabledControlObjectWord = 0;
    }
    BpObject_SetEnabled((void*)enabledControlObjectWord, 1);
    int needInit = ((int&)Sing_805DC670());
    RequestReadyScriptStarted(self) = 0; // no ready script has run for this text yet
    AsCEventMessageSC(self)->scriptCompletionLatched = 0;
    if (needInit == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (base + 0x18));
            s_bpPlayerInitialized = 1;
        }
        ((int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
    void* playerForText = (void*)((int&)Sing_805DC670());
    // Expansion constructs the event-owned WString at +0x14. The loaded message
    // table is borrowed from the player; only this event's text is replaced.
    MessageTable_ExpandEntryText(*(void**)playerForText, (void**)&this->text, TextRequest(arg).localizedMessageId, 0);
    EnsureStoryPlayer(base);
    void* playerForRenderer = (void*)((int&)Sing_805DC670());
    // Phase 3: create/register a renderer if absent, then reset this request
    // and shared advance input. The event owns text; the registry retains the renderer.
    if ((*(int*)&AsCBpPlayer(playerForRenderer)->messageDraw) == 0) {
        StoryMessageDrawAllocation* made = new StoryMessageDrawAllocation(
            ((StoryLoadedFontView*)AsCBpPlayer(playerForRenderer)->fixedFontRenderer)->fontResource);
        EnsureMessageDrawRegistry(base);
        PointerIntVector_AppendPair((void*)((int&)Sing_805DC650()), made, 1);
        AsCBpPlayer(playerForRenderer)->messageDraw = made;
        ((CMsgDraw*)made)->SetDefaultRevealSound(storyMessageStrings + STORY_STRING_OFFSET(revealSound));
    }
    ((CMsgDraw*)(AsCBpPlayer(playerForRenderer)->messageDraw))->SetColor(0xFF);
    int needInit2 = ((int&)Sing_805DC670());
    AsCEventMessageSC(self)->messageDrawPrepared = 0;
    AsCEventMessageSC(self)->advanceInputState = kStoryInputInitial;
    AsCEventMessageSC(self)->advanceInputRequired = 0;
    AsCEventMessageSC(self)->inputDisabled = TextRequest(arg).disableUserInput;
    if (0 == needInit2) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (base + 0x18));
            s_bpPlayerInitialized = 1;
        }
        ((int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
    (*(int*)&AsCBpPlayer((int&)Sing_805DC670())->advanceRequested) = 0;
}
extern "C" {
#pragma pop

struct Dtor2158 {
    virtual ~Dtor2158();
};

#pragma push
#pragma use_lmw_stmw on
struct CEventMessageSC_MSG_WIN : CEventConte {
    unsigned char command; // 0x4: kStoryWindowClose, kStoryWindowOpen or kStoryWindowAdvance
    char pad_5[3];
    CEventMessageSC_MSG_WIN(unsigned char* arg);
    ~CEventMessageSC_MSG_WIN();
    int FinishWindowScript();
};
// Close/open the story window, or request advancement of its current text.
// Borrowed command bytes: [0] opcode, [1] kStoryWindowClose/Open/Advance.
// Store only one byte at event+4: MSG_WIN's command tag overlaps the MSG_REQ
// ready-script latch in the recovered common view. It is not a four-byte latch here.
// The close/open paths unregister and delete the old renderer before rebuilding.
// base is the destructor-registration block: +0x0C registry, +0x18 player, +0x24 BP.
CEventMessageSC_MSG_WIN::CEventMessageSC_MSG_WIN(unsigned char* arg) {
    void* self = this;
    char* base = s_inputManagerDtorRegistration_StoryEvent;
    *(void**)self = __vt__23CEventMessageSC_MSG_WIN;
    unsigned char type = arg[1];
    (*(unsigned char*)&AsCEventMessageSC(self)->unk_4) = type;
    switch (type) {
    case kStoryWindowClose: {
        EnsureStoryPlayer(base);
        void* child;
        void* obj = (void*)((int&)Sing_805DC670());
        child = AsCBpPlayer(obj)->messageDraw;
        if (child) {
            if (child) {
                EnsureMessageDrawRegistry(base);
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), child);
                if (child) {
                    delete (Dtor2158*)child;
                }
            }
            AsCBpPlayer(obj)->messageDraw = 0;
        }
        EnsureStoryPlayer(base);
        void* obj2 = (void*)((int&)Sing_805DC670());
        AsCBpPlayer(obj2)->messageAlpha = 0;
        EventMessage_RebuildMessageDraw(obj2);
        EnsureStoryPlayer(base);
        void* obj3 = (void*)((int&)Sing_805DC670());
        void* path = ((AsCBpPlayer(obj3)->windowOutScriptFlags) >> 31) ? AsCBpPlayer(obj3)->windowOutScriptData
                                                                       : (char*)(AsCBpPlayer(obj3)->stringBytes_4C + 1);
        int id = AsCBpPlayer(obj3)->controlEntryId;
        int result = 0;
        if (id != -1) {
            EnsureBpControl(base);
            result = IdMap_Lookup(StoryControlMap(), id);
        }
        BpObject_StartScriptByName(result, path, 1);
        break;
    }
    case kStoryWindowOpen: {
        EnsureStoryPlayer(base);
        void* child;
        void* obj = (void*)((int&)Sing_805DC670());
        child = AsCBpPlayer(obj)->messageDraw;
        if (child) {
            if (child) {
                EnsureMessageDrawRegistry(base);
                PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), child);
                if (child) {
                    delete (Dtor2158*)child;
                }
            }
            AsCBpPlayer(obj)->messageDraw = 0;
        }
        EnsureStoryPlayer(base);
        void* obj2 = (void*)((int&)Sing_805DC670());
        AsCBpPlayer(obj2)->messageAlpha = 0;
        EventMessage_RebuildMessageDraw(obj2);
        EnsureStoryPlayer(base);
        void* obj3 = (void*)((int&)Sing_805DC670());
        void* path = ((AsCBpPlayer(obj3)->windowInScriptFlags) >> 31) ? AsCBpPlayer(obj3)->windowInScriptData
                                                                      : (char*)(AsCBpPlayer(obj3)->stringBytes_58 + 1);
        int id = AsCBpPlayer(obj3)->controlEntryId;
        int result = 0;
        if (id != -1) {
            EnsureBpControl(base);
            result = IdMap_Lookup(StoryControlMap(), id);
        }
        BpObject_StartScriptByName(result, path, 1);
        break;
    }
    case kStoryWindowAdvance:
        EnsureStoryPlayer(base);
        (*(int*)&AsCBpPlayer((int&)Sing_805DC670())->advanceRequested) = 1;
        break;
    }
}

#pragma pop

// Initialize the story-message angle constant to one full turn in radians.
void StoryMessage_InitializeAngleConstant() {
    s_twoPi_800A2158 = kFloat_6_2831855;
}

// Install the party item interface and initialize its three payload words.
void PartyIItem_ctor(void* self, int payload0, int payload1, int payload2) {
    AsCEventMessageSC(self)->unk_4 = payload1;
    *(int*)self = (int)__vt__Q25party5IItem;
    AsCEventMessageSC(self)->scriptCompletionLatched = payload2;
    AsCEventMessageSC(self)->messageDrawPrepared = payload0;
}

// The window command owns no resources.
CEventMessageSC_MSG_WIN::~CEventMessageSC_MSG_WIN() {}

// The wide message text is released by the WString member's destructor.
CEventMessageSC_MSG_REQ::~CEventMessageSC_MSG_REQ() {}

// Wait for window scripting to complete, then clear speaker and script state after a close.
int CEventMessageSC_MSG_WIN::FinishWindowScript() {
    EnsureRequestStoryPlayer();
    void* obj = (void*)((int&)Sing_805DC670());
    int result;
    if (AsCBpPlayer(obj)->controlEntryId != kStoryMissingControlEntry) {
        EnsureRequestStoryControl();
        result = IdMap_Lookup(StoryControlMap(), AsCBpPlayer(obj)->controlEntryId);
    } else {
        result = 0;
    }
    // Phase 1: wait for the current window script before changing speaker/string state.
    if (!BpObject_IsCurrentScriptComplete(result))
        return 0;
    // Only the CLOSE tag (command == kStoryWindowClose) clears speaker and transition strings.
    // OPEN and ADVANCE complete after their script without this teardown.
    do {
        if (command != kStoryWindowClose)
            break;

        EnsureRequestStoryPlayer();
        obj = (void*)((int&)Sing_805DC670());
        StoryMessage_SelectTransitionScripts(obj, -1);
        AsCBpPlayer(obj)->speakerIndex = -1;
        AsCBpPlayer(obj)->speakerTableIndex = -1;
        AsCBpPlayer(obj)->speakerSlotIndex = -1;
        EventMessage_RebuildMessageDraw(obj);

        EnsureRequestStoryPlayer();
        obj = (void*)((int&)Sing_805DC670());

        // Phase 2 (close only): empty each tagged string in place. Heap capacity is retained;
        // inline strings clear their length bits and first character without reallocating.
        if (!(AsCBpPlayer(obj)->messageRequestScriptFlags >> 31)) {
            AsCBpPlayer(obj)->stringBytes_40[1] = 0;
            {
                int _t = AsCBpPlayer(obj)->stringBytes_40[0];
                _t &= ~0x7F;
                AsCBpPlayer(obj)->stringBytes_40[0] = (unsigned char)_t;
            }
        } else {
            *(unsigned char*)AsCBpPlayer(obj)->messageRequestScriptData = 0;
            (*(int*)&AsCBpPlayer(obj)->stringLength_44) = 0;
        }
        if (!(AsCBpPlayer(obj)->windowOutScriptFlags >> 31)) {
            AsCBpPlayer(obj)->stringBytes_4C[1] = 0;
            {
                int _t = AsCBpPlayer(obj)->stringBytes_4C[0];
                _t &= ~0x7F;
                AsCBpPlayer(obj)->stringBytes_4C[0] = (unsigned char)_t;
            }
        } else {
            *(unsigned char*)AsCBpPlayer(obj)->windowOutScriptData = 0;
            (*(int*)&AsCBpPlayer(obj)->stringLength_50) = 0;
        }
        if (!(AsCBpPlayer(obj)->windowInScriptFlags >> 31)) {
            AsCBpPlayer(obj)->stringBytes_58[1] = 0;
            {
                int _t = AsCBpPlayer(obj)->stringBytes_58[0];
                _t &= ~0x7F;
                AsCBpPlayer(obj)->stringBytes_58[0] = (unsigned char)_t;
            }
        } else {
            *(unsigned char*)AsCBpPlayer(obj)->windowInScriptData = 0;
            (*(int*)&AsCBpPlayer(obj)->stringLength_5C) = 0;
        }
        if (!(AsCBpPlayer(obj)->messageReadyScriptFlags >> 31)) {
            AsCBpPlayer(obj)->stringBytes_64[1] = 0;
            {
                int _t = AsCBpPlayer(obj)->stringBytes_64[0];
                _t &= ~0x7F;
                AsCBpPlayer(obj)->stringBytes_64[0] = (unsigned char)_t;
            }
        } else {
            *(unsigned char*)AsCBpPlayer(obj)->messageReadyScriptData = 0;
            (*(int*)&AsCBpPlayer(obj)->stringLength_68) = 0;
        }
        if (!(AsCBpPlayer(obj)->messageAdvanceScriptFlags >> 31)) {
            AsCBpPlayer(obj)->stringBytes_70[1] = 0;
            {
                int _t = AsCBpPlayer(obj)->stringBytes_70[0];
                _t &= ~0x7F;
                AsCBpPlayer(obj)->stringBytes_70[0] = (unsigned char)_t;
            }
        } else {
            *(unsigned char*)AsCBpPlayer(obj)->messageAdvanceScriptData = 0;
            (*(int*)&AsCBpPlayer(obj)->stringLength_74) = 0;
        }
    } while (0);

    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462064", _ctors_u800A2158_0, StoryMessage_InitializeAngleConstant);
#pragma pop
