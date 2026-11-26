// Tour hint window: the two-page tip box shown during a tour. It builds a heading, up to three
// value/message rows and digit-marked text from a table of values.
// TourHint_ShowFormattedText fills the pages; the WStr helpers at the top read the UTF-16 string layout.

#include <decomp/static_init.h>
extern "C" {
extern unsigned char s_tourHintTextPositionGroupNames[]; // Hint text-position group names.
extern unsigned char s_tourHintRowGroups[];              // Hint row groups and digit-markup pointers.
extern unsigned char s_tourHintStateMessages[];          // Hint-state message pointers.
}
#include "classes/TourHintMessageTable.h"
#include "classes/CMsgDraw.h"
#include "classes/CTourHintWindow.h"

#include "game/singletons.h"
#include "sdk_protos.h"

struct TourTipsGroupNames {
    char nextIndicator[7];  // R_next
    char messageWindow[8];  // msg_win
    char caddiePortrait[8]; // R_cadie
    char pipinPortrait[8];  // R_pipin
};

// Address-space view: tables follow the 14-byte layout-name symbol; they are not owned by it.
struct TourHintResourceAddressView {
    char prefix[0x88];
    int textPositionGroups[4];
    char gap098[0x178 - 0x98];
    int rowGroups[9];
    char gap19C[4];
    wchar_t* digitTags[10];
};

#define HINT_RESOURCE_OFFSET(field) ((int)&((TourHintResourceAddressView*)0)->field)
#define HINT_OFFSET(field) ((int)&((TourHintWindowState*)0)->field)

extern "C" {

// These integer aliases preserve the 32-bit ABI while distinguishing editable roles.
typedef int HintTextHandle;
typedef int HintFontHandle;
typedef int HintBpObjectId;

struct TourHintWindowState {
    void* vtable;
    int nextIndicatorEnabled;
    unsigned char padding008[0x0C];
    int revealProgress;
    unsigned char padding18[4];
    HintTextHandle textExpander;
    HintTextHandle headingText;
    HintTextHandle rowText[3]; // one formatted line per input triple
    HintFontHandle font;
    int textColor;
    unsigned char padding38[0x10];
    HintBpObjectId pageBpObjectIds[2]; // Per-page BP object IDs; -1 means absent.
    unsigned int index;
    unsigned char padding54[4];
    int formattedTextVisible;
    unsigned char padding5C[4];
    int updateNextIndicator;
};

struct TourHintValues {
    unsigned char padding00[0x28];
    signed char state;
    unsigned char padding29[1];
    // Three {primary number, message-table index, secondary number} triples.
    // Message indices <= 0 hide that row; positive indices must be within stateMessageText[8].
    // Secondary values are divided by 5 unless the caller keeps the original scale.
    // The gameplay unit of either number is not established by this formatter.
    short values[9];
};

// 12-byte UTF-16 small string. Heap form uses size bit 31, len and data;
// inline form stores length in the first byte (low 7 bits) and characters at +2.
// Inline text aliases the descriptor: never read data/len as pointers/counts then.
struct WStr {
    int size;
    int len;
    wchar_t* data;
};

struct Int4 {
    int v[4];
};

extern TourHintWindowState s_tourHintWindow;
// BP controller storage is opaque here; it has no hint-window member layout.
extern unsigned char s_bpCtrl[];
extern signed char s_tourHintWindowInitialized;
extern signed char s_bpCtrlInitialized;
extern float kFloat_6_2831855;
extern float s_tourHintTextFullTurn;
extern char s_tourHintWindowDtorRegistration_802FE[];
extern char s_tourTipsBpCtrlDtorRegistration[];
extern TourTipsGroupNames tourTipsWindowStrings;
extern char tourHintLayoutName[];
extern char tourHintWideFormatFragments[];
extern int tourTipsDefaultScriptNames[];
extern int s_tourHintGlyphScaleX;
extern int s_tourHintGlyphScaleY;
extern const char* tourInfoBaseGroupNames[];

extern void CTourHintWindow_dtor(void*, int);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*);
extern int IdMap_Lookup(void*, int);
extern int BpObject_GetOrCreateNamedGroup(int, void*);
extern void UIGroup_SetEnabled(int, int);
extern void BpObject_TryInvokeCurrentCallback(int, int);
extern void
BpObject_Draw(int); /* draws the object passed in r3 (see name_entry_soft_keyboard.cpp, story_message_window.cpp) */
extern void BpObject_SetEnabled(int, int);
extern void UIGroup_SetOpacity(int, int);
extern void BpObject_StartScriptByName(int, int, int);
extern void UIGroup_GetPosition(int, void*);
extern void IntArray_DivideByFiveIfFlagClear(
    void*, int,
    int); /* (values, count, keepOriginalValueScale): defined in minigame_aim_and_party_tiers.cpp; non-zero keepOriginalValueScale skips the divide by 5 */
extern void WString_ReplaceRange(void*, int, int, const void*, const void*, void*);
extern void Text_ExpandTaggedText(void*, void*, const void*, int);
extern void itoa(int, char*, int);
extern void WFixedFont_CreateAndRegisterMsgDraw(int, void*, int);
extern void Text_SetPosition(int, void*);
extern void Text_SetContent(int, void*);
extern void appFreeUni__FPv(void*);

// Publish the shared hint window, constructing it and registering static cleanup once.
static inline void EnsureTourHintWindow() {
    if (((TourHintWindowState*&)Singleton_TourHintWindow()) == 0) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)&s_tourHintWindow)->__ct();
            __register_global_object(&s_tourHintWindow, (void*)CTourHintWindow_dtor,
                                     s_tourHintWindowDtorRegistration_802FE);
            s_tourHintWindowInitialized = 1;
        }
        ((TourHintWindowState*&)Singleton_TourHintWindow()) = &s_tourHintWindow;
    }
}

// Publish the BP lookup controller, constructing it and registering static cleanup once.
static inline void EnsureBpController() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_tourTipsBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Return the ownership bit of the inline/heap UTF-16 string representation.
static inline unsigned int WHASHEAP(const WStr& s) {
    return ((unsigned)s.size) >> 31;
}
// Return UTF-16 code-unit length; the first byte stores inline length on this big-endian ABI.
static inline int WLEN(const WStr& s) {
    return !WHASHEAP(s) ? (*(unsigned char*)&s & 0x7F) : s.len;
}
// Borrow the active UTF-16 characters; valid until this string next mutates.
static inline wchar_t* WCHARS(WStr& s) {
    return !WHASHEAP(s) ? (wchar_t*)((char*)&s + 2) : s.data;
}

// Append one digit-tag fragment, preserving the caller's distinct allocator-tag bytes.
static inline void AppendHintDigitTag(WStr& formattedRow, wchar_t* src, int& allocatorTag, int& allocatorTagSource) {
    int appendOffset = WLEN(formattedRow);
    int n;
    *(char*)&allocatorTag = *(char*)&allocatorTagSource;
    n = (int)wcslen(src);
    WString_ReplaceRange(&formattedRow, appendOffset, 0, src, src + n, &allocatorTag);
}

// Borrow pointer and code-unit count together; inline descriptor words alias UTF-16 text.
static inline void SelectHintWideSpan(WStr& text, wchar_t*& characters, int& length) {
    if (!WHASHEAP(text)) {
        characters = (wchar_t*)((char*)&text + 2);
        length = *(unsigned char*)&text & 0x7F;
    } else {
        characters = text.data;
        length = text.len;
    }
}

enum HintFormattingConstants {
    kHintRowCount = 3,
    kDecimalDigitCount = 10,
    kHintPageCount = 2,
    kCaddieHintPage = 0,
    kPipinHintPage = 1,
    kAbsentHintBpObject = -1,
    kHintGroupsPerRow = 3,
    kHintRowGroupCount = 9,
    kHintInfoGroupCount = 14,
    kDecimalRadix = 10,
    kRowSeparatorByteOffset = 0x26
};

#pragma push
#pragma use_lmw_stmw on
// Reset both hint pages, expand the heading and up to three value/message rows, and create their text objects.
// hint is borrowed for this call; state and positive row message indices must fit the 8-entry table.
// For visible rows both numbers (secondary AFTER optional division by five)
// must be nonnegative: decimal conversion indexes digitTags[character - '0'].
// Negative text would index before the table; this routine supplies no validation.
// Page 0 uses the caddie portrait; page 1 uses Pipin and the three formatted rows.
// values[page] is a BP object ID (-1 = absent). Text positions are copied from layout groups.
void TourHint_ShowFormattedText(void* unusedContext, const TourHintValues* hint, int keepOriginalValueScale) {
    int rowGroupByteOffset;
    int transparentTextColor;
    char* formatFragments;
    int textExpanderHandle;
    int* primaryValue;
    int* secondaryValue;
    int* rowPositionGroup;
    int rowTextByteOffset;
    char primaryDigits[100];
    char secondaryDigits[100];
    const char* infoGroupNames[14];
    wchar_t* primaryDigitTags[10];
    wchar_t* secondaryDigitTags[10];
    int enabledRowGroups[9];
    int hiddenRowGroups[9];
    int stateMessages[8];
    int rowMessageKinds[3];
    int primaryValues[3];
    int secondaryValues[3];
    WStr formattedRow;
    WStr messageText;
    WStr suffixText;
    Int4 headingGroupNames;
    int rowGroupNames[4];
    int headingGlyphScale[2];
    int headingPosition[2];
    int rowGlyphScale[2];
    int rowPosition[2];
    // Seven source/destination pairs for empty allocator-tag byte copies, not hint values.
    // The source bytes intentionally have no initialized payload in the original library ABI.
    // Keep their distinct stack lifetimes: initializing or merging them can change generated code.
    int clearMessageTag, clearMessageTagSource, clearRowTag, clearRowTagSource;
    int primaryDigitTag, primaryDigitTagSource, messageTagSource, messageTag;
    int separatorTagSource, separatorTag, secondaryDigitTag, secondaryDigitTagSource;
    int suffixTagSource, suffixTag;
    TourHintWindowState* win;
    int bpObjectOrGroup;
    int pageOrRowIndex;
    int tableElement;

    // Phase 1: transpose the three packed triples into parallel row arrays and select message text.
    rowMessageKinds[0] = hint->values[1];
    rowMessageKinds[1] = hint->values[4];
    rowMessageKinds[2] = hint->values[7];
    primaryValues[0] = hint->values[0];
    primaryValues[1] = hint->values[3];
    primaryValues[2] = hint->values[6];
    // Original tourHintLayoutName +0x340..+0x35C: text selected by hint->state below.
    stateMessages[0] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[0];
    stateMessages[1] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[1];
    stateMessages[2] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[2];
    stateMessages[3] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[3];
    stateMessages[4] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[4];
    stateMessages[5] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[5];
    stateMessages[6] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[6];
    stateMessages[7] = ((int*)((char*)(&((TourHintMessageTable*)tourHintLayoutName)->stateMessageText[0])))[7];
    secondaryValues[0] = hint->values[2];
    secondaryValues[1] = hint->values[5];
    secondaryValues[2] = hint->values[8];
    IntArray_DivideByFiveIfFlagClear(
        secondaryValues, 3,
        keepOriginalValueScale); /* 0x802FFA18: r5 = the incoming keepOriginalValueScale (tour_character_map_board.cpp passes !flags->bit), never written here */

    // Phase 2: reset both available BP pages, hide old groups, restart scripts and callbacks.
    EnsureTourHintWindow();
    win = ((TourHintWindowState*&)Singleton_TourHintWindow());
    for (pageOrRowIndex = 0; pageOrRowIndex < kHintPageCount; pageOrRowIndex++) {
        if (win->pageBpObjectIds[pageOrRowIndex] != kAbsentHintBpObject) {
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            BpObject_SetEnabled(bpObjectOrGroup, 1);
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            if (bpObjectOrGroup == 0) {
                bpObjectOrGroup = 0;
            } else {
                bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.nextIndicator);
            }
            if (bpObjectOrGroup != 0) {
                UIGroup_SetOpacity(bpObjectOrGroup, 0);
            }
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            if (bpObjectOrGroup == 0) {
                bpObjectOrGroup = 0;
            } else {
                bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.messageWindow);
            }
            if (bpObjectOrGroup != 0) {
                UIGroup_SetOpacity(bpObjectOrGroup, 0);
            }
            switch (pageOrRowIndex) {
            case 0:
                EnsureBpController();
                bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kCaddieHintPage]);
                if (bpObjectOrGroup == 0) {
                    bpObjectOrGroup = 0;
                } else {
                    bpObjectOrGroup =
                        BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.caddiePortrait);
                }
                if (bpObjectOrGroup != 0) {
                    UIGroup_SetOpacity(bpObjectOrGroup, 0);
                }
                break;
            case 1:
                EnsureBpController();
                bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
                if (bpObjectOrGroup == 0) {
                    bpObjectOrGroup = 0;
                } else {
                    bpObjectOrGroup =
                        BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.pipinPortrait);
                }
                if (bpObjectOrGroup != 0) {
                    UIGroup_SetOpacity(bpObjectOrGroup, 0);
                }
                break;
            }
            if (pageOrRowIndex == 1) {
                for (tableElement = 0; tableElement < kHintInfoGroupCount; tableElement++) {
                    infoGroupNames[tableElement] = tourInfoBaseGroupNames[tableElement];
                }
                for (tableElement = 0; tableElement < kHintInfoGroupCount; tableElement++) {
                    EnsureBpController();
                    bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
                    if (bpObjectOrGroup == 0) {
                        bpObjectOrGroup = 0;
                    } else {
                        bpObjectOrGroup =
                            BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, (void*)infoGroupNames[tableElement]);
                    }
                    if (bpObjectOrGroup != 0) {
                        UIGroup_SetOpacity(bpObjectOrGroup, 0);
                    }
                }
            }
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            if (bpObjectOrGroup == 0) {
                bpObjectOrGroup = 0;
            } else {
                bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.nextIndicator);
            }
            if (bpObjectOrGroup != 0) {
                UIGroup_SetEnabled(bpObjectOrGroup, 0);
            }
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            BpObject_StartScriptByName(bpObjectOrGroup, tourTipsDefaultScriptNames[pageOrRowIndex], 1);
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[pageOrRowIndex]);
            BpObject_TryInvokeCurrentCallback(bpObjectOrGroup, 0);
        }
    }

    // Phase 3: reset reveal progress and refresh the currently selected page.
    win->updateNextIndicator = 0;
    EnsureTourHintWindow();
    ((TourHintWindowState*&)Singleton_TourHintWindow())->revealProgress = 0;
    EnsureTourHintWindow();
    win = ((TourHintWindowState*&)Singleton_TourHintWindow());
    if (win->pageBpObjectIds[win->index] != kAbsentHintBpObject) {
        if (win->updateNextIndicator != 0) {
            EnsureBpController();
            bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[win->index]);
            if (bpObjectOrGroup == 0) {
                bpObjectOrGroup = 0;
            } else {
                bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, tourTipsWindowStrings.nextIndicator);
            }
            if (bpObjectOrGroup != 0) {
                UIGroup_SetEnabled(bpObjectOrGroup, win->nextIndicatorEnabled);
            }
        }
        EnsureBpController();
        bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[win->index]);
        BpObject_TryInvokeCurrentCallback(bpObjectOrGroup, 0);
    }

    // Phase 4: an inactive state hides formatted text; active states rebuild heading and rows.
    if (hint->state <= 0) {
        EnsureTourHintWindow();
        ((TourHintWindowState*&)Singleton_TourHintWindow())->formattedTextVisible = 0;
    } else {
        formattedRow.size = 0;
        formattedRow.len = 0;
        formattedRow.data = 0;
        messageText.size = 0;
        messageText.len = 0;
        messageText.data = 0;
        suffixText.size = 0;
        suffixText.len = 0;
        suffixText.data = 0;

        struct StringCleanup {
            WStr* string;
            // Free only a completed heap string when leaving the formatting scope.
            ~StringCleanup() {
                if (WHASHEAP(*string))
                    appFreeUni__FPv(string->data);
            }
        };
        StringCleanup formattedRowCleanup = {&formattedRow};
        StringCleanup messageCleanup = {&messageText};
        StringCleanup suffixCleanup = {&suffixText};

        EnsureTourHintWindow();
        textExpanderHandle = ((TourHintWindowState*&)Singleton_TourHintWindow())->textExpander;
        Text_ExpandTaggedText((void*)textExpanderHandle, &suffixText, tourHintWideFormatFragments, 0);
        Text_ExpandTaggedText((void*)textExpanderHandle, &messageText, (void*)stateMessages[hint->state], 0);

        // Build the heading at textPositionGroups[0]; the font registers its renderer in the window.
        EnsureTourHintWindow();
        headingGroupNames = *(Int4*)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(textPositionGroups))));
        win = ((TourHintWindowState*&)Singleton_TourHintWindow());
        EnsureBpController();
        bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
        if (bpObjectOrGroup == 0) {
            bpObjectOrGroup = 0;
        } else {
            bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, (void*)headingGroupNames.v[0]);
        }
        UIGroup_GetPosition(bpObjectOrGroup, headingPosition);
        headingGlyphScale[0] = s_tourHintGlyphScaleX;
        headingGlyphScale[1] = s_tourHintGlyphScaleY;
        win->textColor = 0;
        WFixedFont_CreateAndRegisterMsgDraw(win->font, (char*)win + HINT_OFFSET(headingText), 1);
        Text_SetPosition(win->headingText, headingPosition);
        ((CMsgDraw*)(win->headingText))->ResetTextState();
        ((CMsgDraw*)(win->headingText))->ClearFormattingAndRevealAll();
        ((CMsgDraw*)(win->headingText))->SetGlyphScale((float*)headingGlyphScale);
        ((CMsgDraw*)(win->headingText))->SetColor(win->textColor);
        ((CMsgDraw*)win->headingText)->colorMultiplier = win->textColor;
        {
            wchar_t* src;
            src = WCHARS(messageText);
            Text_SetContent(win->headingText, src);
        }

        EnsureTourHintWindow();
        win = ((TourHintWindowState*&)Singleton_TourHintWindow());
        win->formattedTextVisible = 1;
        primaryValue = primaryValues;
        secondaryValue = secondaryValues;
        rowPositionGroup = rowGroupNames;
        rowTextByteOffset = 0;
        transparentTextColor = 0;
        formatFragments = tourHintWideFormatFragments;
        rowGroupByteOffset = 0;
        for (pageOrRowIndex = 0; pageOrRowIndex < kHintRowCount; pageOrRowIndex++) {
            if (rowMessageKinds[pageOrRowIndex] > 0) {
                // Row recipe: tagged primary digits + expanded message + separator
                // + tagged secondary digits + expanded suffix. Allocator bytes below are ABI tags.
                // A. Clear reusable strings and expand this row's selected message.
                *(char*)&clearMessageTag = *(char*)&clearMessageTagSource;
                WString_ReplaceRange(&messageText, 0, -1, 0, 0, &clearMessageTag);
                *(char*)&clearRowTag = *(char*)&clearRowTagSource;
                WString_ReplaceRange(&formattedRow, 0, -1, 0, 0, &clearRowTag);
                Text_ExpandTaggedText((void*)textExpanderHandle, &messageText,
                                      (void*)stateMessages[rowMessageKinds[pageOrRowIndex]], 0);
                for (tableElement = 0; tableElement < kDecimalDigitCount; tableElement++) {
                    primaryDigitTags[tableElement] = *(
                        wchar_t**)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(digitTags))) + 4 * tableElement);
                }
                // B. Append the primary number as per-digit tagged fragments.
                itoa(primaryValue[0], primaryDigits, kDecimalRadix);
                for (tableElement = 0; tableElement < (int)strlen(primaryDigits); tableElement++) {
                    int c = (signed char)primaryDigits[tableElement];
                    wchar_t* src = primaryDigitTags[c - '0'];
                    AppendHintDigitTag(formattedRow, src, primaryDigitTag, primaryDigitTagSource);
                }
                // C. Append expanded message, then the fixed UTF-16 separator.
                {
                    int appendOffset = WLEN(formattedRow);
                    wchar_t* src;
                    int n;
                    // Resolve pointer and length together: inline words alias text.
                    SelectHintWideSpan(messageText, src, n);
                    *(char*)&messageTag = *(char*)&messageTagSource;
                    WString_ReplaceRange(&formattedRow, appendOffset, 0, src, src + n, &messageTag);
                }
                {
                    int appendOffset = WLEN(formattedRow);
                    wchar_t* src = (wchar_t*)(formatFragments + kRowSeparatorByteOffset);
                    int n;
                    *(char*)&separatorTag = *(char*)&separatorTagSource;
                    n = (int)wcslen(src);
                    WString_ReplaceRange(&formattedRow, appendOffset, 0, src, src + n, &separatorTag);
                }
                for (tableElement = 0; tableElement < kDecimalDigitCount; tableElement++) {
                    secondaryDigitTags[tableElement] = *(
                        wchar_t**)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(digitTags))) + 4 * tableElement);
                }
                // D. Append the scaled secondary number, then the expanded suffix.
                itoa(secondaryValue[0], secondaryDigits, kDecimalRadix);
                for (tableElement = 0; tableElement < (int)strlen(secondaryDigits); tableElement++) {
                    int c = (signed char)secondaryDigits[tableElement];
                    wchar_t* src = secondaryDigitTags[c - '0'];
                    AppendHintDigitTag(formattedRow, src, secondaryDigitTag, secondaryDigitTagSource);
                }
                {
                    int appendOffset = WLEN(formattedRow);
                    wchar_t* src;
                    int n;
                    // Same inline/heap span selection as the expanded message.
                    SelectHintWideSpan(suffixText, src, n);
                    *(char*)&suffixTag = *(char*)&suffixTagSource;
                    WString_ReplaceRange(&formattedRow, appendOffset, 0, src, src + n, &suffixTag);
                }

                // Register rowText[pageOrRowIndex] at textPositionGroups[pageOrRowIndex+1] and enable its three BP groups.
                EnsureTourHintWindow();
                win = ((TourHintWindowState*&)Singleton_TourHintWindow());
                for (tableElement = 0; tableElement < 4; tableElement++) {
                    rowGroupNames[tableElement] =
                        *(int*)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(textPositionGroups))) +
                                4 * tableElement);
                }
                EnsureBpController();
                bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
                if (bpObjectOrGroup == 0) {
                    bpObjectOrGroup = 0;
                } else {
                    bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(bpObjectOrGroup, (void*)rowPositionGroup[1]);
                }
                UIGroup_GetPosition(bpObjectOrGroup, rowPosition);
                rowGlyphScale[0] = s_tourHintGlyphScaleX;
                rowGlyphScale[1] = s_tourHintGlyphScaleY;
                win->textColor = transparentTextColor;
                WFixedFont_CreateAndRegisterMsgDraw(
                    win->font, (char*)win + HINT_OFFSET(headingText) + (pageOrRowIndex + 1) * 4, 1);
                Text_SetPosition(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText)), rowPosition);
                ((CMsgDraw*)(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText))))->ResetTextState();
                ((CMsgDraw*)(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText))))
                    ->ClearFormattingAndRevealAll();
                ((CMsgDraw*)(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText))))
                    ->SetGlyphScale((float*)rowGlyphScale);
                ((CMsgDraw*)(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText))))->SetColor(win->textColor);
                ((CMsgDraw*)*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText)))->colorMultiplier =
                    win->textColor;
                {
                    wchar_t* src;
                    if (!WHASHEAP(formattedRow)) {
                        src = (wchar_t*)((char*)&formattedRow + 2);
                    } else {
                        src = formattedRow.data;
                    }
                    Text_SetContent(*(int*)((char*)win + rowTextByteOffset + HINT_OFFSET(rowText)), src);
                }

                EnsureTourHintWindow();
                win = ((TourHintWindowState*&)Singleton_TourHintWindow());
                for (tableElement = 0; tableElement < kHintRowGroupCount; tableElement++) {
                    enabledRowGroups[tableElement] =
                        *(int*)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(rowGroups))) + 4 * tableElement);
                }
                for (tableElement = 0; tableElement < kHintGroupsPerRow; tableElement++) {
                    EnsureBpController();
                    bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
                    if (bpObjectOrGroup == 0) {
                        bpObjectOrGroup = 0;
                    } else {
                        bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(
                            bpObjectOrGroup, (void*)enabledRowGroups[(rowGroupByteOffset >> 2) + tableElement]);
                    }
                    if (bpObjectOrGroup != 0) {
                        UIGroup_SetEnabled(bpObjectOrGroup, 1);
                    }
                }
            } else {
                EnsureTourHintWindow();
                win = ((TourHintWindowState*&)Singleton_TourHintWindow());
                for (tableElement = 0; tableElement < kHintRowGroupCount; tableElement++) {
                    hiddenRowGroups[tableElement] =
                        *(int*)(((char*)(tourHintLayoutName + HINT_RESOURCE_OFFSET(rowGroups))) + 4 * tableElement);
                }
                for (tableElement = 0; tableElement < kHintGroupsPerRow; tableElement++) {
                    EnsureBpController();
                    bpObjectOrGroup = IdMap_Lookup(Sing_805DC5F0(), win->pageBpObjectIds[kPipinHintPage]);
                    if (bpObjectOrGroup == 0) {
                        bpObjectOrGroup = 0;
                    } else {
                        bpObjectOrGroup = BpObject_GetOrCreateNamedGroup(
                            bpObjectOrGroup, (void*)hiddenRowGroups[(rowGroupByteOffset >> 2) + tableElement]);
                    }
                    if (bpObjectOrGroup != 0) {
                        UIGroup_SetEnabled(bpObjectOrGroup, 0);
                    }
                }
            }
            // One row advances one renderer slot (4 bytes) and three group IDs (12 bytes).
            primaryValue++;
            secondaryValue++;
            rowPositionGroup++;
            rowTextByteOffset += 4;
            rowGroupByteOffset += 12;
        }
    }
}
#pragma pop

// Initialize this tour-hint unit's full-turn constant.
void TourHintText_InitFullTurn() {
    s_tourHintTextFullTurn = kFloat_6_2831855;
}

} // extern "C"

// Apply the next-indicator state when requested and run the current page callback.
// Returns 1 even when no page object is present; it is not a callback result.
// The shared window singleton is used; the receiver is not read.
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
int CTourHintWindow::UpdateNextIndicatorAndCallback() {
    CTourHintWindow* obj;
    int layoutIndex;
    char* nextIndicatorGroupName;
    int data;
    int result;
    int layoutObject;
    EnsureTourHintWindow();
    obj = ((CTourHintWindow*&)Singleton_TourHintWindow());
    layoutIndex = obj->mode;
    if (obj->layoutIds[layoutIndex] != kAbsentHintBpObject) {
        if (obj->updateNextIndicator) {
            data = obj->nextIndicatorEnabled;
            nextIndicatorGroupName = tourTipsWindowStrings.nextIndicator;
            EnsureBpController();
            result = IdMap_Lookup(Sing_805DC5F0(), obj->layoutIds[layoutIndex]);
            if (result == 0) {
                result = 0;
            } else {
                result = BpObject_GetOrCreateNamedGroup(result, nextIndicatorGroupName);
            }
            if (result) {
                UIGroup_SetEnabled(result, data);
            }
        }
        EnsureBpController();
        layoutObject = IdMap_Lookup(Sing_805DC5F0(), obj->layoutIds[obj->mode]);
        BpObject_TryInvokeCurrentCallback(layoutObject, 0);
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma pop

// Draw the selected hint page when its BP object id is valid.
// The shared window singleton is used; the receiver is not read.
void CTourHintWindow::DrawCurrentLayout() {
    EnsureTourHintWindow();
    CTourHintWindow* obj = ((CTourHintWindow*&)Singleton_TourHintWindow());
    if (obj->layoutIds[obj->mode] != kAbsentHintBpObject) {
        EnsureBpController();
        BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), obj->layoutIds[obj->mode]));
    }
}

// Step back 0x18 bytes to the start of the window, then run its destructor.
void CTourHintWindow::DtorThunk18(int deleting) {
    CTourHintWindow_dtor((char*)this - 0x18, deleting);
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625B0", _ctors_u802FF968_0, TourHintText_InitFullTurn);
#pragma pop
