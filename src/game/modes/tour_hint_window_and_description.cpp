// Tour hint descriptions: builds the text for a board-node tip, such as minigame rules, targets and
// conditions, or round details with the enemy, hole count and course, in the tour hint window.
// Also holds the computer-menu selection base and the helpers that manage the global hint window.

#include <decomp/static_init.h>

#include "classes/CComSelectBase.h"
#include "classes/MWStringStorage.h"
#include "classes/CFade.h"
#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CPangyaEnemyData.h"
#include "classes/CTourMessage.h"
#include "classes/CTourHintWindow.h"
#include "classes/CBpCtrl.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpPlayer.h"
#include "classes/CMsgDraw.h"

// Node data used to choose and format tour-board descriptions.
enum {
    kDescriptionNewlineOffset = 0xec
};
enum TourNodeKind {
    kRoundNode,
    kStartNode,
    kHintNode,
    kMinigameNode,
    kWarpNode
};
enum TourHintPhase {
    kHintIdle,
    kHintFadeIn,
    kHintReadText,
    kHintFadeOut
};
// Caller-owned phase: 0 idle -> 1 fade-in -> 2 text/input -> 3 fade-out.
// Update returns 1 when phase 3 completes and restores phase 0. Each fade update
// advances by 1 in CFade's update units; the selected fade step is 0x20.
// ShowDescriptionHint prepares content at phase 0; StartGlobalFadeIn starts it.
enum {
    kTourHoleCount = 18,
    kNoDescriptionEntry = -1,
    kDescriptionFadeStep = 0x20
};
struct TourDescriptionContext {
    int hintPhase;
    void* textContext;
};
struct TourDescriptionNode {
    char reserved00[0xa];
    signed char kind;
    char reserved0B[0x1d];
    unsigned char minigameTextId;
    char reserved29;
    short targetValue;
    short conditionTextId;
    char reserved2E[0xe];
    short courseTextId;
    short enemyIndex;
    char reserved40[2];
    unsigned char ruleTextId;
    char reserved43;
    signed char enabledHoles[kTourHoleCount];
};
struct TourHintFadeView {
    char reserved00[0x38];
    CFade fade;
};
struct TourHintOpacityView {
    char reserved00[0x3c];
    unsigned char opacity;
};
struct ChatMessageContextView {
    char reserved00[0x10];
    void* messageContext;
};
struct MessageFontView {
    char reserved00[0xac];
    void* font;
};
struct EnemyDescriptionView {
    unsigned int reserved00;
    unsigned int nameTextId;
};
struct BpHandlePair {
    int entryId;
    int scriptIndex;
};
// Context/node pointers are borrowed. Output strings are already constructed
// Metrowerks wide strings (16-bit code units); these routines mutate their contents.
// Node kinds 0..4 map to round/start/hint/minigame/warp. Round fields: enemyIndex
// and courseTextId use -1 for omission, ruleTextId is a message tag index, and
// any nonzero enabledHoles byte contributes one to the displayed hole count.
// Minigame fields: minigameTextId 1/2 selects the extra condition header;
// conditionTextId 4..6 appends a condition; targetValue must format as digits
// only because each ASCII character indexes a ten-entry localized digit table.

extern "C" {

extern unsigned char s_tourHintWindow[];
extern signed char s_tourHintWindowInitialized;
extern char s_recordTourHintWindowDtorRegistration[];

extern unsigned char s_bpPlayer[];
extern signed char s_bpPlayerInitialized;
extern char s_bpPlayerDtorRegistration_TourMenu[];
extern void __dt__9CBpPlayerFv();
extern void TourMessageOwner_DestroyMessageDraw(void*);
extern void TourHintMessage_SetText(void*, void*);
extern unsigned char s_pangyaEnemyData[];
extern char s_tourDescriptionEnemyDataDtorRegistration[];
extern signed char s_pangyaEnemyDataInitialized;
extern void __dt__16CPangyaEnemyDataFv();
extern void* Table_GetEntry(void*, int);
extern void StoryMessage_ExpandCharacterNameTag(void*, void*, int);
extern void TourPosition_FormatDescription(void*, void*, void*);
extern void TourPosition_BuildDescriptionText(void*, void*, void*);
extern void appFreeUni__FPv(void*);

extern float kFloat_0_ComputerMenuInputDelay;
extern unsigned char __vt__14CComSelectBase[];
// Packed view of the existing string storage; member lengths include terminators.
struct ComputerMenuStringPool {
    char textureArchive[10];
    char layoutFile[10];
    char menuBp[10];
    char backgroundBp[17];
    char titleBp[21];
    char comvs_44[6];
    char bg_4A[3];
    char title_4D[6];
    char R_bg_title_font2_53[17];
    char R_bg_title_font1_64[17];
    char text_75[19];
    char ui_page_change_88[15];
    char ui_button_cancel_click_97[23];
    char R_MSG_title_K13_LC_AE[19];
    char basic_string_reserve_length_error_C1[35];
    char std_exception_std_logic_error_std_length_erro_E4[55];
    char basic_string_out_of_range_11B[27];
    char std_exception_std_logic_error_std_out_of_rang_136[55];
    char basic_string_length_error_16D[27];
};
#define ComputerMenuStringPoolView (*(ComputerMenuStringPool*)computerVersusMenuStrings)
extern unsigned char computerVersusMenuStrings[];
extern unsigned char s_bpCtrl[];
extern char s_comSelectResourceTableDtorRegistration[];
extern char s_bpCtrlRegistration_8052FED4[];
extern void* s_resourceTableStorage;
extern signed char s_resourceTableInitialized;
extern signed char s_bpCtrlInitialized;
extern unsigned char* g_chatMsg;
extern void MPBase1_Ctor(void*);
extern void* ResourceTable_FindEntryByName(void*, void*, void*);
extern void* __nw__FUl(unsigned int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, void*);
extern void Allocation_Dtor();
extern void CBpCtrl_dtor();
extern float kFloat_6_2831855;
extern float s_comSelectRotationPeriod;
extern float kFloat_1_RoundScene;

extern void CTourHintWindow_dtor();

extern unsigned int s_tourMinigameConditionTextTemplates[];
extern unsigned int s_tourDescriptionDigitText[];
// Packed view of the existing string storage; member lengths include terminators.
struct TourDescriptionStringPool {
    wchar_t startDescription[30];
    wchar_t hintDescription[29];
    wchar_t warpDescription[29];
    wchar_t roundTitle[30];
    wchar_t newline[2];
    wchar_t roundRule[30];
    wchar_t roundHoleCount[30];
    wchar_t holeSuffix[3];
    wchar_t roundCourse[30];
    wchar_t minigameTitle[33];
    wchar_t minigameTarget[33];
};
#define TourDescriptionStringPoolView (*(TourDescriptionStringPool*)tourPositionMessageStrings)
extern char tourPositionMessageStrings[];
extern void Text_ExpandTaggedText(void*, void*, void*, int);
extern void WString_ReplaceRange(void*, int, int, void*, void*, void*);
extern void itoa(int, void*, int);
extern void StoryMessage_ExpandAlternateNameTag(void*, void*, int);
extern void StoryMessage_ExpandContextTag(void*, void*, int);

extern void TourHint_InvokeCurrentPageCallback(void*);
}

namespace app {
template <class T> class app_allocator {};
}
namespace std {
template <class T> class char_traits {};
template <class T, class Traits, class Allocator> class basic_string {
public:
    unsigned int flags;
    unsigned int size;
    void* data;
    // Initialize the empty inline-string representation.
    basic_string() {
        flags = 0;
        size = 0;
        data = 0;
    }
    // Release only heap-backed storage; inline text belongs to this object.
    ~basic_string() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
};
}

extern "C" {
// Ensure singleton storage and its shutdown registration exist; do not cache its address across calls.
static inline void EnsureTourHintWindow() {
    if (((unsigned char*&)Singleton_TourHintWindow()) == 0) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, (void*)CTourHintWindow_dtor,
                                     s_recordTourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned char*&)Singleton_TourHintWindow()) = (unsigned char*)s_tourHintWindow;
    }
}

// Decode the already-loaded string tag into a wide-code-unit append index.
// The tag load remains at the caller's original point; no buffer ownership changes.
static inline void DecodeDescriptionAppendIndex(unsigned int& index, void* string, unsigned int tag) {
    if ((tag >> 31) == 0)
        index = *(unsigned char*)string & 0x7f;
    else
        index = AsMWStringStorage(string)->length;
}

#pragma opt_lifetimes off
// Choose a board-node description and display it in the global tour hint window.
void TourPosition_ShowDescriptionHint(void* contextStorage, void* nodeStorage) {
    void* context = contextStorage;
    void* node = nodeStorage;

    std::basic_string<wchar_t, std::char_traits<wchar_t>, app::app_allocator<wchar_t> > description;
    unsigned char* bpPlayer = ((unsigned char*&)Sing_805DC670());

    // Phase 1: release the old message and reset the singleton window to page zero.
    if (bpPlayer == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned char*&)Sing_805DC670()) = (unsigned char*)s_bpPlayer;
        bpPlayer = ((unsigned char*&)Sing_805DC670());
    }
    TourMessageOwner_DestroyMessageDraw((unsigned char*&)Sing_805DC670());

    EnsureTourHintWindow();
    ((CTourHintWindow*)((unsigned char*&)Singleton_TourHintWindow()))->ResetPageScripts();

    EnsureTourHintWindow();
    ((CTourHintWindow*)(((unsigned char*&)Singleton_TourHintWindow())))->SelectPage(0);

    EnsureTourHintWindow();
    ((CFade*)((unsigned char*)&((TourHintFadeView*)Singleton_TourHintWindow())->fade))->SetStep(kDescriptionFadeStep);

    EnsureTourHintWindow();
    ((CTourMessage*)(((unsigned char*&)Singleton_TourHintWindow())))->SetInputMode(1);

    // Phase 2: choose the node-specific description. Unknown kinds keep an empty string.
    signed char nodeKind = ((TourDescriptionNode*)node)->kind;
    *(int*)context = kHintIdle;

    switch (nodeKind) {
    case kRoundNode:
        TourPosition_FormatDescription(context, node, &description);
        break;
    case kStartNode:
        Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, &description,
                              (void*)tourPositionMessageStrings, 0);
        break;
    case kHintNode:
        Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, &description,
                              (void*)(TourDescriptionStringPoolView.hintDescription), 0);
        break;
    case kMinigameNode:
        TourPosition_BuildDescriptionText(context, node, &description);
        break;
    case kWarpNode:
        Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, &description,
                              (void*)(TourDescriptionStringPoolView.warpDescription), 0);
        break;
    default:
        break;
    }

    EnsureTourHintWindow();
    // Phase 3: pass the temporary string to the window before its local buffer is destroyed.
    TourHintMessage_SetText(((unsigned char*&)Singleton_TourHintWindow()), &description);
}
#pragma opt_lifetimes reset

// Append ABI: string bit 31 chooses heap length at +4, otherwise the first
// byte's low seven bits hold the inline length. Lengths/indices count 16-bit
// code units; ReplaceRange takes a byte-addressed [begin,end) source range.
// ByteSlot is the original allocator-argument storage, rounded to a word. Its
// volatile source byte is intentionally unspecified; never interpret it as text,
// a character count or a node flag. The callee consumes it synchronously.
struct ByteSlot80320A90 {
    unsigned char value;
    char pad[3];
};

struct ConditionTextTable {
    unsigned int value[8];
};

struct DigitTextTable {
    unsigned int value[10];
};

#pragma push
#pragma use_lmw_stmw on
// Build a minigame description from its type, rule, target value and condition text.
void TourPosition_BuildDescriptionText(void* contextStorage, void* nodeStorage, void* outputString) {
    void* context = contextStorage;
    void* description = outputString;
    void* node = nodeStorage;
    char* text;
    unsigned int appendPosition;
    ConditionTextTable conditionText;
    DigitTextTable digitText;
    char targetDigits[100];

    // Phase 1: title and optional condition header, then newline and rule tag.
    conditionText = *(ConditionTextTable*)s_tourMinigameConditionTextTemplates;

    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.minigameTitle), 0);

    if ((unsigned char)(((TourDescriptionNode*)node)->minigameTextId - 1) <= 1) {
        Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                              (void*)conditionText.value[(signed char)((TourDescriptionNode*)node)->minigameTextId], 0);
    }

    {
        unsigned int stringFlags = *(unsigned int*)description;
        volatile ByteSlot80320A90 src;
        ByteSlot80320A90 dst;
        int len;

        text = tourPositionMessageStrings + kDescriptionNewlineOffset;
        DecodeDescriptionAppendIndex(appendPosition, description, stringFlags);
        dst.value = src.value;
        len = wcslen((const wchar_t*)text);
        WString_ReplaceRange(description, appendPosition, 0, text, (void*)(text + (len << 1)), &dst);
    }

    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.roundRule), 0);

    if (((unsigned char*&)Sing_805DC670()) == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned char*&)Sing_805DC670()) = (unsigned char*)s_bpPlayer;
    }
    StoryMessage_ExpandAlternateNameTag(((unsigned char*&)Sing_805DC670()), description,
                                        ((TourDescriptionNode*)node)->ruleTextId);

    {
        unsigned int stringFlags = *(unsigned int*)description;
        volatile ByteSlot80320A90 src;
        ByteSlot80320A90 dst;
        int len;

        text = tourPositionMessageStrings + kDescriptionNewlineOffset;
        DecodeDescriptionAppendIndex(appendPosition, description, stringFlags);
        dst.value = src.value;
        len = wcslen((const wchar_t*)text);
        WString_ReplaceRange(description, appendPosition, 0, text, (void*)(text + (len << 1)), &dst);
    }

    // Phase 2: target label and localized decimal digits; append trailing condition if supported.
    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.minigameTarget), 0);

    {
        ByteSlot80320A90 dst;
        volatile ByteSlot80320A90 src;
        unsigned char ch;
        unsigned int i;
        int len;

        digitText = *(DigitTextTable*)s_tourDescriptionDigitText;
        itoa(((TourDescriptionNode*)node)->targetValue, targetDigits, 10);

        ch = src.value;
        i = 0;
        while (i < (unsigned int)strlen(targetDigits)) {
            signed char digit = targetDigits[i];
            unsigned int stringFlags;

            text = (char*)digitText.value[digit - '0'];
            stringFlags = *(unsigned int*)description;
            DecodeDescriptionAppendIndex(appendPosition, description, stringFlags);
            dst.value = ch;
            len = wcslen((const wchar_t*)text);
            WString_ReplaceRange(description, appendPosition, 0, text, (void*)(text + (len << 1)), &dst);
            i++;
        }
    }

    if ((unsigned short)(((TourDescriptionNode*)node)->conditionTextId - 4) <= 2) {
        Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                              (void*)conditionText.value[((TourDescriptionNode*)node)->conditionTextId], 0);
    }
}
#pragma pop

struct ByteSlot803205F4 {
    unsigned char value;
    char pad[3];
};
struct DigitTable803205F4 {
    unsigned int value[10];
};

#pragma push
#pragma use_lmw_stmw on
// Build a round description including enemy, rule, enabled-hole count and course.
void TourPosition_FormatDescription(void* contextStorage, void* nodeStorage, void* outputString) {
    void* context = contextStorage;
    void* description = outputString;
    void* node = nodeStorage;
    // Three machine words retain their matched lifetime groups. Active roles:
    // holeCountThenDigitAddress: enabled-hole count until itoa, then ASCII byte cursor;
    // enemyAddressThenAllocatorByte: borrowed enemy record until name expansion,
    // then the allocator argument byte copied to each localized-digit append;
    // indexThenSuffixAddress: append length, then digit index, then wide suffix address.
    // The explicit casts below are representation transitions, not numeric conversions.
    register int holeCountThenDigitAddress = 0;
    unsigned int indexThenSuffixAddress;
    unsigned char* text;
    unsigned int enemyAddressThenAllocatorByte;

    {
        // Phase 1: count selected holes; negative bytes count as enabled too.
        unsigned char* p = (unsigned char*)((TourDescriptionNode*)node)->enabledHoles;
        for (int offset = 0; offset < kTourHoleCount; p++) {
            if ((signed char)*p != 0)
                holeCountThenDigitAddress++;
            offset++;
        }
    }

    // Phase 2: round title, optional enemy name, newline, and rule description.
    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.roundTitle), 0);

    if (((TourDescriptionNode*)node)->enemyIndex != -1) {
        if (((unsigned char*&)Sing_805DC678()) == 0) {
            if (!s_pangyaEnemyDataInitialized) {
                text = s_pangyaEnemyData;
                ((CPangyaEnemyData*)text)->Init();
                __register_global_object(text, (void*)__dt__16CPangyaEnemyDataFv,
                                         s_tourDescriptionEnemyDataDtorRegistration);
                s_pangyaEnemyDataInitialized = 1;
            }
            ((unsigned char*&)Sing_805DC678()) = (unsigned char*)s_pangyaEnemyData;
        }
        enemyAddressThenAllocatorByte = (unsigned int)Table_GetEntry(*(void**)((unsigned char*&)Sing_805DC678()),
                                                                     ((TourDescriptionNode*)node)->enemyIndex);
        if (((unsigned char*&)Sing_805DC670()) == 0) {
            if (!s_bpPlayerInitialized) {
                text = s_bpPlayer;
                ((CBpPlayer*)text)->__ct();
                __register_global_object(text, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
                s_bpPlayerInitialized = 1;
            }
            ((unsigned char*&)Sing_805DC670()) = (unsigned char*)s_bpPlayer;
        }
        StoryMessage_ExpandCharacterNameTag(
            ((unsigned char*&)Sing_805DC670()), description,
            (unsigned char)((EnemyDescriptionView*)enemyAddressThenAllocatorByte)->nameTextId);
    }

    {
        unsigned int stringTag = *(unsigned int*)description;
        text = (unsigned char*)TourDescriptionStringPoolView.newline;
        DecodeDescriptionAppendIndex(indexThenSuffixAddress, description, stringTag);
        volatile ByteSlot803205F4 source0;
        ByteSlot803205F4 byte0;
        byte0.value = source0.value;
        int textLength = wcslen((const wchar_t*)text);
        WString_ReplaceRange(description, indexThenSuffixAddress, 0, text, (void*)(text + (textLength << 1)),
                             (void*)&byte0);
    }

    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.roundRule), 0);

    if (((unsigned char*&)Sing_805DC670()) == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned char*&)Sing_805DC670()) = (unsigned char*)s_bpPlayer;
    }
    StoryMessage_ExpandAlternateNameTag(((unsigned char*&)Sing_805DC670()), description,
                                        ((TourDescriptionNode*)node)->ruleTextId);

    // Phase 3: localize the hole-count digits and suffix, then the optional course name.
    // itoa consumes the count; only after that call may the word become a digit cursor.
    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.roundHoleCount), 0);

    {
        text = (unsigned char*)s_tourDescriptionDigitText;
        DigitTable803205F4 w2 = *(DigitTable803205F4*)text;
        char buf[100];
        itoa(holeCountThenDigitAddress, buf, 10);
        ByteSlot803205F4 byteC;
        volatile ByteSlot803205F4 sourceC;
        enemyAddressThenAllocatorByte = sourceC.value;
        holeCountThenDigitAddress = (int)buf;
        indexThenSuffixAddress = 0;
        while (indexThenSuffixAddress < (unsigned int)strlen(buf)) {
            signed char digit = *(signed char*)holeCountThenDigitAddress;
            text = (unsigned char*)w2.value[digit - '0'];
            unsigned int stringTag = *(unsigned int*)description;
            unsigned int appendPosition;
            DecodeDescriptionAppendIndex(appendPosition, description, stringTag);
            byteC.value = enemyAddressThenAllocatorByte;
            int textLength = wcslen((const wchar_t*)text);
            WString_ReplaceRange(description, appendPosition, 0, text, (void*)(text + (textLength << 1)),
                                 (void*)&byteC);
            holeCountThenDigitAddress++;
            indexThenSuffixAddress++;
        }
    }

    {
        unsigned int stringTag = *(unsigned int*)description;
        // Digit iteration is finished: this word now holds a wide-string address.
        indexThenSuffixAddress = (unsigned int)(TourDescriptionStringPoolView.holeSuffix);
        unsigned int appendPosition;
        DecodeDescriptionAppendIndex(appendPosition, description, stringTag);
        volatile ByteSlot803205F4 source;
        ByteSlot803205F4 byte;
        byte.value = source.value;
        int len = wcslen((const wchar_t*)indexThenSuffixAddress);
        WString_ReplaceRange(description, appendPosition, 0, (void*)indexThenSuffixAddress,
                             (void*)(indexThenSuffixAddress + (len << 1)), &byte);
    }
    Text_ExpandTaggedText(((TourDescriptionContext*)context)->textContext, description,
                          (void*)(TourDescriptionStringPoolView.roundCourse), 0);

    if (((TourDescriptionNode*)node)->courseTextId != -1) {
        if (((unsigned char*&)Sing_805DC670()) == 0) {
            if (!s_bpPlayerInitialized) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
                s_bpPlayerInitialized = 1;
            }
            ((unsigned char*&)Sing_805DC670()) = (unsigned char*)s_bpPlayer;
        }
        StoryMessage_ExpandContextTag(((unsigned char*&)Sing_805DC670()), description,
                                      (unsigned char)((TourDescriptionNode*)node)->courseTextId);
    }
}

#pragma pop
// Initialize the computer-menu rotation-period value.
void ComSelectBase_InitRotationPeriod() {
    s_comSelectRotationPeriod = kFloat_6_2831855;
}

// Draw the current page of the lazily created global hint window.
void CTourHintWindow::DrawSingletonPage(void* messageHolder) {
    EnsureTourHintWindow();
    ((CTourHintWindow*)((unsigned char*&)Singleton_TourHintWindow()))->DrawCurrentPage();
}

// Clear the message renderer and all objects registered with the hint window.
inline void*& Sing_805DC688() {
    static void* s;
    return s;
}
static inline void EnsureTargetTourHintWindow() {
    if (Sing_805DC688() == 0) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, (void*)CTourHintWindow_dtor,
                                     s_recordTourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        Sing_805DC688() = s_tourHintWindow;
    }
}
void TourHintWindow_ClearGlobalMessageAndObjects() {
    if (((unsigned char*&)Sing_805DC670()) == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_TourMenu);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned char*&)Sing_805DC670()) = s_bpPlayer;
    }
    TourMessageOwner_DestroyMessageDraw((unsigned char*&)Sing_805DC670());

    EnsureTargetTourHintWindow();
    ((CTourHintWindow*)Sing_805DC688())->ClearRegisteredObjects();
}

// Start the hint-window fade and put the caller in the fade-in phase.
void TourHintWindow_StartGlobalFadeIn(int* hintPhase) {
    EnsureTargetTourHintWindow();
    ((CFade*)((unsigned char*)&((TourHintFadeView*)Sing_805DC688())->fade))->StartFadeIn();
    *hintPhase = kHintFadeIn;
}

extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle;
extern char s_computerMenuBpHandleDtorRegistration[];
extern void CBpCtrlHandle_Dtor();
extern void Layout_Destroy(void*, int);
extern void __dl__FPv(void*);

typedef CComSelectBase Obj80320DD0;

// Initialize computer-opponent selection and load its layout, BP layers and text renderer.
// A named member rather than a constructor: callers run it on an object a derived class already owns.
void* CComSelectBase::Init() {
    MPBase1_Ctor(this);

    float f0 = kFloat_0_ComputerMenuInputDelay;
    vtbl = (void*)__vt__14CComSelectBase;
    bpEntryId = -1;
    selectedEntry = -1;
    pageIndex = -1;
    *(int*)pad_024 = 0;
    inputDelay = f0;
    requestedPageIndex = 0;
    textureResource = 0;
    layout = 0;
    textRenderer = 0;
    optionDialog = 0;
    optionDialogPending = 0;
    courseClearDialog = 0;
    courseClearNotificationPending = 0;
    secondaryBpEntryId = -1;
    tertiaryBpEntryId = -1;
    int archiveBytes = 0;

    // Phase 1: request the texture archive and allocate the owned layout.
    void* resourceTable = Sing_805DC620();
    if (resourceTable == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     s_comSelectResourceTableDtorRegistration);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = (void*)&s_resourceTableStorage;
    }

    void* archiveHandle =
        ResourceTable_FindEntryByName(Sing_805DC620(), ComputerMenuStringPoolView.textureArchive, &archiveBytes);
    // Construct the archive loader in-place with new-expression exception cleanup.
    struct RealNew_fn_80320DD0_0 {
        char storage[0x20];
        RealNew_fn_80320DD0_0(__typeof__(computerVersusMenuStrings) a0, __typeof__(archiveBytes) a1,
                              __typeof__(archiveHandle) a2) {
            ((CPackTextureResource*)this)->Init((const char*)a0, (void**)a1, (unsigned int)a2);
        }
    };
    void* resourceOrLayout = new RealNew_fn_80320DD0_0((unsigned char*)ComputerMenuStringPoolView.textureArchive,
                                                       archiveBytes, archiveHandle);

    textureResource = resourceOrLayout;

    resourceOrLayout = __nw__FUl(0x5c);
    if (resourceOrLayout) {
        Layout_Init(resourceOrLayout, 5);
    }
    layout = resourceOrLayout;
    Layout_LoadExtended(resourceOrLayout, 0, ComputerMenuStringPoolView.layoutFile);

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Phase 2: register menu/background/title layers; destructor removes all three IDs.
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(ComputerMenuStringPoolView.menuBp, &bpEntryId, 0);

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(ComputerMenuStringPoolView.backgroundBp, &secondaryBpEntryId, 0);

    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((void*)((unsigned char*)ComputerMenuStringPoolView.titleBp), &tertiaryBpEntryId, 0);

    // Phase 3: create the owned text renderer using a borrowed font from the chat context.
    void* messageContext = ((ChatMessageContextView*)g_chatMsg)->messageContext;
    void* renderer = __nw__FUl(0x4bc);
    if (renderer) {
        void* font = ((MessageFontView*)messageContext)->font;
        ((CMsgDraw*)renderer)->__ct(font);
    }
    textRenderer = renderer;

    return this;
}

struct Virtual80321098 {
    virtual ~Virtual80321098();
};

// Release selection layouts, resources, BP registrations and owned dialog objects.
void* __dt__15SelectMatchBaseFv(void* self, int dealloc) {
    // Positive dealloc deletes self; zero/negative performs member cleanup only.
    if (self != 0) {
        Obj80320DD0* o = (Obj80320DD0*)self;
        void* bpHandle = Sing_805DC5F8();
        o->vtbl = (void*)__vt__14CComSelectBase;
        if (bpHandle == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle = -1;
                ((BpHandlePair*)&s_bpCtrlHandle)->scriptIndex = -1;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                         s_computerMenuBpHandleDtorRegistration);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = &s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();

        void* layout = o->layout;
        if (layout != 0) {
            Layout_Destroy(layout, 1);
            o->layout = 0;
        }

        void* resource = o->textureResource;
        if (resource != 0) {
            delete (Virtual80321098*)resource;
            o->textureResource = 0;
        }

        void* textRenderer = o->textRenderer;
        if (textRenderer != 0) {
            delete (Virtual80321098*)textRenderer;
            o->textRenderer = 0;
        }

        void* optionDialog = o->optionDialog;
        if (optionDialog != 0) {
            delete (Virtual80321098*)optionDialog;
            o->optionDialog = 0;
        }

        if (o->bpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(o->bpEntryId);
            o->bpEntryId = -1;
        }

        if (o->secondaryBpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(o->secondaryBpEntryId);
            o->secondaryBpEntryId = -1;
        }

        if (o->tertiaryBpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(o->tertiaryBpEntryId);
            o->tertiaryBpEntryId = -1;
        }

        // This auxiliary object is owned by the selector and uses the same virtual delete slot as its UI.
        void* auxiliaryObject = o->courseClearDialog;
        if (auxiliaryObject != 0) {
            delete (Virtual80321098*)auxiliaryObject;
            o->courseClearDialog = 0;
        }

        if (dealloc > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Advance hint fade-in, message input and fade-out; return one when dismissal completes.
int TourHint_UpdateFadeAndInput(void* phaseStorage) {
    int* phase = (int*)phaseStorage;
    int dismissed = 0;

    // Advance exactly one state per call; input completion starts, but does not tick, fade-out.
    switch (*phase) {
    case kHintIdle:
        break;
    case kHintFadeIn: {
        EnsureTourHintWindow();
        if (((CFade*)(((unsigned char*)(unsigned char*)&((TourHintFadeView*)Singleton_TourHintWindow())->fade)))
                ->Update(kFloat_1_RoundScene) != 0) {
            *phase = kHintReadText;
        }
        break;
    }
    case kHintReadText: {
        EnsureTourHintWindow();
        if (((CTourMessage*)(((unsigned char*&)Singleton_TourHintWindow())))->ProcessInput(1) != 0) {
            EnsureTourHintWindow();
            ((CFade*)(((unsigned char*)(unsigned char*)&((TourHintFadeView*)Singleton_TourHintWindow())->fade)))
                ->StartFadeOut();
            *phase = kHintFadeOut;
        }
        break;
    }
    case kHintFadeOut: {
        EnsureTourHintWindow();
        if (((CFade*)(((unsigned char*)(unsigned char*)&((TourHintFadeView*)Singleton_TourHintWindow())->fade)))
                ->Update(kFloat_1_RoundScene) != 0) {
            *phase = kHintIdle;
            dismissed = 1;
        }
        break;
    }
    default:
        break;
    }

    EnsureTourHintWindow();
    // Synchronize navigation and glyph alpha, then run the current page callback even when idle.
    unsigned char opacity = ((TourHintOpacityView*)Singleton_TourHintWindow())->opacity;

    EnsureTourHintWindow();
    ((CTourHintWindow*)(((unsigned char*&)Singleton_TourHintWindow())))->SetNavigationOpacity(opacity);

    EnsureTourHintWindow();
    ((CTourMessage*)(((unsigned char*&)Singleton_TourHintWindow())))->ApplyGlyphColorMask(opacity);

    EnsureTourHintWindow();
    TourHint_InvokeCurrentPageCallback((unsigned char*&)Singleton_TourHintWindow());

    return dismissed;
}

// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(TourDescriptionContext, textContext, 0x4);
POLISH_OFFSET(TourDescriptionNode, kind, 0xa);
POLISH_OFFSET(TourDescriptionNode, minigameTextId, 0x28);
POLISH_OFFSET(TourDescriptionNode, courseTextId, 0x3c);
POLISH_OFFSET(TourDescriptionNode, ruleTextId, 0x42);
POLISH_OFFSET(TourDescriptionNode, enabledHoles, 0x44);
POLISH_OFFSET(TourHintFadeView, fade, 0x38);
POLISH_OFFSET(TourHintOpacityView, opacity, 0x3c);
POLISH_OFFSET(MessageFontView, font, 0xac);
#undef POLISH_OFFSET

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625FC", _ctors_u8031FD4C_0, ComSelectBase_InitRotationPeriod);
#pragma pop
