#include <decomp/static_init.h>
// Tutorial text resources, shared golf Tips dialogs and their BP/text rendering.
// Tutorial_LoadMessageResources opens tutorial_msg.ecb through WFixedFont's message
// resource factory; this selects text, not Swing/Button input mappings. Native
// actions are decoded in game_input_manager_cursor_rumble.cpp and consumed by the golf-rule/controller code.
// Shared layouts: CInGameDialog/CInGameFadeDialog/CSmallCommonDialog, CSharedDoc,
// CGolfDoc, CPangyaMsgResourceObject. Scalar/lifetime pool: u802579FC_data.cpp.
// Modder lookup:
// - Tutorial wording: kTutorialMessagesOffset selects tutorial_msg.ecb in
//   golfTipsResourceStrings; TutorialDialog_CreateForMessageIndex expands a
//   zero-based message index. Replacing text cannot change the physical controls.
// - Dialog timing: kFloat_0_2_CommonDialogFadeSeconds=0.2 seconds for tutorial;
//   golf Tips default to kFloat_0_5_GolfTipsFadeSeconds=0.5 fade seconds and
//   kFloat_2_GolfTipsHoldSeconds=2 hold seconds. Per-show arguments can override.
//   These durations differ from FadeState's integer update-call durations.
// - Widescreen/font placement: CInGameFadeDialog_Draw reads R_MSG_tips_line1
//   (kTipsLineGroupOffset) for the text anchor; the BP resource supplies the XY.
//   Its alpha scale is 128 for BP and 255 for text. Keep BP/text placement and
//   opacity conversion together; this file has no camera aspect or pin coordinate.
// - GolfTips_ResetActiveDialogForRoundState filters by round flags and script
// - Tip-group overrides use party mode 2 (Driving Contest table in course_select_menu.cpp).
//   Some symbols below say Darts but serve the Driving Contest.
// Script/message/group lookup, indexed by golfTipsDialogNames (15 pointers at
// 0x804B37B8; exact spellings include the original "COM Turm" typo):
// ID  BP script                  automatic close?  mode-2 rest / total-score
//  0  Approach Shot               yes              default / default
//  1  For Birdie                  yes              default / default
//  2  For Eagle                   yes              default / default
//  3  For Win                     yes              default / default
//  4  Your Turn                   yes              hidden / shown
//  5  Unplayable Area             no               hidden / hidden
//  6  Unplayable Area_titl_only   no               hidden / hidden
//  7  Time Out                    no               default / default
//  8  1P Turn                     yes              hidden / shown
//  9  2P Turn                     yes              hidden / shown
// 10  3P Turn                     yes              hidden / shown
// 11  4P Turn                     yes              hidden / shown
// 12  COM Turm                    yes              default / default
// 13  Dormie Hole                 yes              default / default
// 14  Sudden death                yes              default / default
// Automatic means eligible for RequestClose(-2), NOT automatically opened.
// Its search order is 4,8,9,10,11,12,0,1,2,3,13,14 (s_automaticTipScriptIndices).
// Explicit RequestClose(0..14) targets one name; other invalid values close any
// active script. Indexed Show requires 0..14; -2 is NOT a valid show request.
// Wording is independent of script: all IDs use R_MSG_tips_line1, with nullable
// caller-supplied UTF-16 tagged text. There is no fixed script->message-index
// conversion. Producers in golf_rule_round_flow_and_shot_sim.cpp: For Win/Birdie/Eagle/turn/approach text
// comes from golfRuleMessageTags (T7A); timeout is +0x44, unplayable is +0x78.
// golf_player_animation_and_model_loads.cpp supplies kCaddieEnabledTipText for ID 5; unplayable_ball_drop_sequence.cpp shows ID 6 with
// no text. Tutorial message indices instead use tutorial_msg.ecb below.
// Visibility group names are R_tips_rest/R_tips_totalscore at +0x1A/+0x26 in
// golfTipsResourceStrings. Defaults are in each BP script, not this override list.
// Preserve reference counts/registry sequencing, duplicate-script rejection,
// message-index validation and string comparison order. Shared names stay as ABI
// spellings; local names below describe observed operations without guessing.
#include "classes/CSharedDoc.h"
// Borrow shared round-selection state. Direct cast expansion retains document
// reload scheduling across SharedDoc_IsPlayerFlag0Set in the round-state gates.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
// Borrow active-round fields; keep the same cast form for those reloads.
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
// Translation unit 0x802579FC-0x80259080 (29 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CSmallCommonDialog.h"
#include "classes/CInGameFadeDialog.h"
#include "classes/CInGameDialog.h"
#include "classes/CPangyaMsgResourceObject.h"
#include "classes/CTask.h"

// Missing portions of the base dialog/string layout; shared class fields stay authoritative.
// Narrow strings keep up to eleven inline bytes starting one byte after the flag.
union DialogScriptName {
    struct Heap { unsigned int flags, length; const char* data; } heap;
    struct Inline { unsigned char length; char text[11]; } inPlace;
};
struct DialogTimingView {
    int referenceCount;
    char baseStorage[0x18];
    float fadeSeconds; // base update divides elapsed time by this in states 3 and 5
    char elapsedAndOpacity[8];
    float holdSeconds; // positive values automatically close state 4 after this duration
    char controlStorage[0xC];
    // +38: nonzero excludes this dialog from RequestEligibleClose's bulk-close
    // sweep (ingame_dialog_and_fade_dialog.cpp); +34 is a separate blocker. It is not a text mode.
    // Explicit RequestClose and UpdateTransition's hold timer do not test +38.
    int blockRegistryClose;
};
struct FadeDialogScriptView {
    char baseStorage[0x50];
    DialogScriptName scriptName;
};
struct DialogVisibilityOverride {
    DialogScriptName groupName;
    void* enabled;
};
struct DialogOverrideVector {
    char* data;
    int count;
    unsigned int capacity;
    char allocatorStorage[4];
    int firstIndex;
};
typedef char DialogScriptNameSize[(sizeof(DialogScriptName) == 12) ? 1 : -1];
typedef char DialogOverrideSize[(sizeof(DialogVisibilityOverride) == 16) ? 1 : -1];

enum GolfTipConstants {
    kMatchCurrentTip = -2,
    kTipScriptCount = 15,
    kAutomaticTipCount = 12,
    kTipVisibilityOverridesPartyMode2 = 2,
    // Round-state meanings follow CGolfRule's dispatch in golf_rule_round_flow_and_shot_sim.cpp.
    kBeginButtonSwingState = 0x10,
    kCameraAwaitInputState = 0x8000,
    kBlockingRoundStates = kBeginButtonSwingState | kCameraAwaitInputState,
    kNonLocalPlayerBlockingState = 0x200, // Camera transition; blocks only if player flag 0 is clear.
    kDialogFullyOpaque = 255,
    kSmallDialogMode5 = 5,
    kTipsLineGroupOffset = 0x9, // R_MSG_tips_line1
    kDrivingContestRestGroupOffset = 0x1A, // R_tips_rest
    kDrivingContestTotalScoreGroupOffset = 0x26, // R_tips_totalscore
    kTutorialMessagesOffset = 0x38, // tutorial_msg.ecb
    kBlockedStateAllowedScriptPointerOffset = 0x14 // script 5, five 4-byte Wii pointers
};
// Shared tips lifetime (registry and globals hold separate references):
// CreateDialogResources -> configure fade/hold/text group and message resource.
// ShowIndexedMessage -> reject duplicate/transition -> close and unregister old
//   script -> replace script/arguments -> open -> add to registry -> retain out.
// SetValueForMatchingTip -> change fade time and request close; no release here.
// UpdateRegisteredDialogs -> request eligible closes, tick/release completed entries.
// ReleaseDialogAndMessageResource -> drop the two global references and null them.
// ResetSharedRegistryAndResources -> clear registry NODES and null globals; it
//   does not perform those two decrements. Keep reset and release roles distinct.
// Lazy singleton blocks publish existing storage AFTER construction/registration;
// their repeated placement is matching scaffolding, not separate dialog owners.
#include "classes/CPartyItemFireBantage.h"

// Borrow party-item fields; construction/destruction keeps its existing base ownership.
static inline CPartyItemFireBantage* AsCPartyItemFireBantage(void* p) { return (CPartyItemFireBantage*)p; }

// Borrow fade-dialog fields; reference ownership is managed by the registry/caller.
static inline CInGameFadeDialog* AsCInGameFadeDialog(void* p) { return (CInGameFadeDialog*)p; }

// Borrow the small-dialog result field without changing its lifetime.
static inline CSmallCommonDialog* AsCSmallCommonDialog(void* p) { return (CSmallCommonDialog*)p; }
struct TipDialogCalls {
	int referenceCount;
	virtual void s0();
	virtual void s1();
	virtual void beginClose(int);
	virtual int isActive();
	virtual void reset();
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_golfTipsTau;
extern float s_fireBantageTau;
extern const float kFloat_0_3_FireBantage;
extern const float kFloat_Neg0_8_FireBantage;
extern const float kFloat_0_CommonDialog;
extern const float kFloat_0_2_CommonDialogFadeSeconds;
extern float kFloat_128_DialogBpOpacity;
extern const float kFloat_0_5_GolfTipsFadeSeconds;
extern const float kFloat_2_GolfTipsHoldSeconds;
extern void* g_golfTipsDialog;
extern void* g_golfTipsMessageResource;
extern void* g_tutorialMessageResource;
extern CTask* g_taskManager;
extern signed char s_refObjectListInitialized;
extern signed char s_bpCtrlInitialized;
extern CSharedDoc* g_sharedDoc;
extern int kDriveRestTipIndex5;
extern int kDriveRestTipIndex6;
extern char s_driveTotalScoreTipIndices[];
extern char s_automaticTipScriptIndices[];
extern char golfTipsDialogNames[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_InGameFadeDialog[];
extern char golfTipsResourceStrings[];
extern char __vt__Q25party21CPartyItemFireBantage[];
extern char s_refObjectList[];
extern char s_refObjectListDtorRegistration_PartyItem[];

int TutorialDialog_PollSharedLayout();
int CInGameTutorialDialog_Create(void*, void**);
void InGameDialog_SetWideText(void*, int);
int MessageContext_GetResourceData(void*);
void MessageTable_ExpandEntryText(void*, void*, int, int);
void RefObjectList_Init(void*);
void CommonDialogRegistry_Remove(void*, void*);
void GolfTipsDialog_SetScriptNameAndClearArguments(void*, void*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void CommonDialogRegistry_AddUnique(void*, void*);
int CInGameFadeDialog_Create(void*, void*);
void* InGameFadeDialog_SetTextGroupName(void*, void*);
int SharedDoc_IsPlayerFlag0Set(unsigned char);
// Return a reference to the smaller string length.
template<class T> inline const T& MinTipLength(const T& a, const T& b) { return a < b ? a : b; }
#pragma dont_inline on
// Return negative/zero/positive ordering for two unsigned tip indices.
int cmp80258(unsigned int a, unsigned int b) {
	if (a < b) return -1;
	if (a == b) return 0;
	return 1;
}
#pragma dont_inline reset
// Look up one of fifteen script names, returning null for an out-of-range index.
inline const char* GetTipScriptName(const char* const* table, int i) {
	const char* r;
	if (i < 0) r = 0;
	else if (i < 0 || (unsigned int)i >= kTipScriptCount) r = 0;
	else r = table[i];
	return r;
}
int GolfTipsDialog_RequestClose(int, float);
void CommonDialogRegistry_UpdateAndReleaseCompleted(void*, float);
void appFreeUni__FPv(void*);
void TutorialLayout_LoadSharedResource();
void TutorialLayout_DestroySharedResource();
int CPangyaMsgResourceObject_Create(void*, void*);
void party_IItem_dtor(void*, int);
void __dl__FPv(void*);
void PartyIItem_ctor(void*, void*, int, int);
void RefObjectList_Init(void*);
void CommonDialogRegistry_ClearNodes(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void* IdMap_Lookup(void*, void*);
void BpObject_SetEnabled(void*, int);
void CInGameDialog_CompleteClose(void*);
void* RefObjectList_ReleaseEntries(void*);
void* CommonDialogRegistry_RequestEligibleClose(void*);
void RefObjectList_dtor();
void __register_global_object(void*, void (*)(), void*);
void InGameFadeDialog_AppendGroupVisibilityOverride(void*, const char*, int);
void UIGroup_SetEnabled(void*, void*);
int BpObject_PollLoad(void*);
void* BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetOpacity(void*, int);
int CSmallCommonDialog_Create(const char*, void**, int);
void CommonDialogRegistry_AddUnique(void*, void*);
void BpCtrl_SetOpacity(void*, int);
int InGameDialog_GetNonemptyWideText(void*);
int Dialog_GetOpacityByte(void*); // opacity byte, derived from the base dialog's fade opacity
void DialogText_DrawAligned(void*, const void*, int, int, int);
int UIGroup_GetPosition(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
unsigned int strlen(const char*);
int memcmp(const char*, const char*, unsigned int);

#pragma push
#pragma use_lmw_stmw on
// Set fade duration and request close on an active tip selected by script index.
// [0,15) requires a matching script; kMatchCurrentTip searches automatic scripts.
// Other invalid indices (notably callers' -1) close ANY active script. A null
// GetTipScriptName result intentionally skips the final name test below.
// fadeSeconds is the replacement fade duration in seconds, including zero.
// Both expanded comparisons implement a lexicographic small-string test:
// compare min(storedLength, requestedLength) bytes, then break ties by length.
// Inline strings use low seven bits of byte zero; heap strings use +4/+8.
// Return 1 after requesting close, 0 if no live/matching tip is found.
int GolfTipsDialog_RequestClose(int index, float fadeSeconds) {
    // Phases: resolve the automatic-tip sentinel if requested, compare the active
    // script with the selected name, then set fade duration and request close.
	if (g_golfTipsDialog == 0) return 0;
	if (g_golfTipsDialog == 0) return 0;
	if (!((TipDialogCalls*)g_golfTipsDialog)->isActive()) return 0;
	// Selection: -2 is restricted to the automatic list, not all fifteen tips.
	if (index == kMatchCurrentTip) {
		char* dialog = (char*)g_golfTipsDialog;
		unsigned int i;
		for (i = 0; i < kAutomaticTipCount; ++i) {
			int candidate = ((int*)s_automaticTipScriptIndices)[i];
			const char* name = GetTipScriptName((const char* const*)golfTipsDialogNames, candidate);
			if (name != 0) {
				unsigned int scriptNameLength;
				unsigned int nameLength = strlen(name);
				unsigned int nameLengthCopy = nameLength;
				scriptNameLength = (((FadeDialogScriptView*)dialog)->scriptName.heap.flags >> 31) == 0 ? (((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f) : ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
				unsigned int storedLengthCopy;
				const char* storedScriptName;
				unsigned int storedLengthRead;
				if (!(((FadeDialogScriptView*)dialog)->scriptName.heap.flags >> 31)) {
					// Keep pointer addition here: a member-array address extends a register lifetime.
					storedScriptName = dialog + (int)&((FadeDialogScriptView*)0)->scriptName.inPlace.text;
					storedLengthRead = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f;
				} else {
					storedScriptName = ((FadeDialogScriptView*)dialog)->scriptName.heap.data;
					storedLengthRead = ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
				}
				storedLengthCopy = storedLengthRead;
				unsigned int storedLengthForComparison = MinTipLength(scriptNameLength, storedLengthCopy);
				unsigned int compareLength = MinTipLength(nameLengthCopy, storedLengthForComparison);
				int cmp = memcmp(storedScriptName, name, compareLength);
				if (cmp == 0) {
					if (storedLengthForComparison < nameLength) cmp = -1;
					else if (storedLengthForComparison == nameLength) cmp = cmp;
					else cmp = 1;
				}
				if (cmp == 0) {
					index = candidate;
					break;
				}
			}
		}
		if (index == kMatchCurrentTip) return 0;
	}
	// Verification: a valid index must still match the live script. The two
	// expanded comparisons above/below differ in address form for exact codegen.
	const char* name = GetTipScriptName((const char* const*)golfTipsDialogNames, index);
	if (name != 0) {
		char* dialog = (char*)g_golfTipsDialog;
		unsigned int scriptNameLength;
		unsigned int nameLength = strlen(name);
		unsigned int nameLengthCopy = nameLength;
		scriptNameLength = (((FadeDialogScriptView*)g_golfTipsDialog)->scriptName.heap.flags >> 31) == 0 ? (((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f) : ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
		unsigned int storedLengthCopy;
		const char* storedScriptName;
		unsigned int storedLengthRead;
		if (!(((FadeDialogScriptView*)dialog)->scriptName.heap.flags >> 31)) {
			storedScriptName = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.text;
			storedLengthRead = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f;
		} else {
			storedScriptName = ((FadeDialogScriptView*)dialog)->scriptName.heap.data;
			storedLengthRead = ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
		}
		storedLengthCopy = storedLengthRead;
		unsigned int storedLengthForComparison = MinTipLength(scriptNameLength, storedLengthCopy);
		unsigned int compareLength = MinTipLength(nameLengthCopy, storedLengthForComparison);
		int cmp = memcmp(storedScriptName, name, compareLength);
		if (cmp == 0) {
			if (storedLengthForComparison < nameLength) cmp = -1;
			else if (storedLengthForComparison == nameLength) cmp = cmp;
			else cmp = 1;
		}
		if (cmp != 0) return 0;
	}
	// Update the fade duration before requesting the dialog close.
	((DialogTimingView*)g_golfTipsDialog)->fadeSeconds = fadeSeconds;
	((TipDialogCalls*)g_golfTipsDialog)->beginClose(1);
	return 1;
}
#pragma pop

#pragma opt_propagation off
// Append rest/total-score visibility overrides for selected mode-2 tip scripts; preserve append order.
// GolfTips_ConfigureDrivingContestTipGroups: mode 2's
// round loader selects the Driving Contest table. Requires a live tips dialog on a
// matching index. Returns 1 when overrides were appended, 0 otherwise. The two
// rest indices append (hidden,hidden); the five other indices append (hidden,shown).
int GolfTips_ConfigureDrivingContestTipGroups(int value) {
	if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != kTipVisibilityOverridesPartyMode2)
		return 0;
	int values[2] = {kDriveRestTipIndex5, kDriveRestTipIndex6};
	char* text = golfTipsResourceStrings;
	char* firstText = text + kDrivingContestRestGroupOffset;
	char* secondText = text + kDrivingContestTotalScoreGroupOffset;
	for (int i = 0; i < 2; ++i) {
		if (value == values[i]) {
			InGameFadeDialog_AppendGroupVisibilityOverride(g_golfTipsDialog, firstText, 0);
			InGameFadeDialog_AppendGroupVisibilityOverride(g_golfTipsDialog, secondText, 0);
			return 1;
		}
	}
	int more[5];
	for (int i = 0; i < 5; ++i) more[i] = ((int*)s_driveTotalScoreTipIndices)[i];
	for (int i = 0; i < 5; ++i) {
		if (value == more[i]) {
			InGameFadeDialog_AppendGroupVisibilityOverride(g_golfTipsDialog, firstText, 0);
			InGameFadeDialog_AppendGroupVisibilityOverride(g_golfTipsDialog, secondText, 1);
			return 1;
		}
	}
	return 0;
}
#pragma opt_propagation reset

#pragma push
#pragma use_lmw_stmw on
// Reset the active tip subject to round-state/script gates; preserve the exact script comparison.
// No global dialog => no action. Blocking flags admit only table entry 5; otherwise
// reset is unconditional. No allocation/reference transfer or viewport write occurs.
void GolfTips_ResetActiveDialogForRoundState() {
	if (g_golfTipsDialog == 0) return;
	if (g_golfTipsDialog == 0) return;
	unsigned int mask;
	mask = (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer) ? 0 : kNonLocalPlayerBlockingState) | kBlockingRoundStates;
	// Blocked states allow reset only for script-table entry 5 (+0x14).
	// The repeated length aliases below implement exact script equality.
	if ((*(unsigned int*)&AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags & mask) != 0) {
	const char* current;
	char* dialog;
	unsigned int scriptNameLength;
	unsigned int nameLength;
	unsigned int nameLengthCopy;
	unsigned int length;
	const char* text;
	unsigned int len;
	unsigned int n;
	unsigned int compareLength;
	int cmp;
	current = *(const char**)(golfTipsDialogNames + kBlockedStateAllowedScriptPointerOffset);
	dialog = (char*)g_golfTipsDialog;
	nameLength = strlen(current);
	nameLengthCopy = nameLength;
	scriptNameLength = (((FadeDialogScriptView*)g_golfTipsDialog)->scriptName.heap.flags >> 31) == 0 ? (((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f) : ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
	if (!(((FadeDialogScriptView*)dialog)->scriptName.heap.flags >> 31)) {
		text = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.text;
		len = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f;
	} else {
		text = ((FadeDialogScriptView*)dialog)->scriptName.heap.data;
		len = ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
	}
	length = len;
	n = MinTipLength(scriptNameLength, length);
	compareLength = MinTipLength(nameLengthCopy, n);
	cmp = memcmp(text, current, compareLength);
	if (cmp == 0) {
		if (n < nameLength) cmp = -1;
		else if (n == nameLength) cmp = cmp;
		else cmp = 1;
	}
	if (cmp != 0) return;
	}
	((TipDialogCalls*)g_golfTipsDialog)->reset();
}
#pragma pop

}
struct RefCountedDialogCalls {
	int referenceCount;
	virtual void destroy(int);
	virtual void f1();
	virtual void f2();
	virtual int isActive();
};
struct TipReadinessCalls {
	int referenceCount;
	virtual void v0();
	virtual void v1();
	virtual void v2();
	virtual int isActive();
	virtual void v4();
	virtual int v5();
};
struct FadeDialogCalls {
	int referenceCount;
	virtual void a0();
	virtual void a1();
	virtual void a2();
	virtual void a3();
	virtual void a4();
	virtual void a5();
	virtual void a6();
	virtual void a7();
	virtual void a8();
	virtual void a9();
	virtual void a10();
	virtual void a11();
	virtual void a12();
	virtual void a13();
	virtual int isReady();
	virtual void a15();
	virtual void drawFade();
};
struct CommonDialogCalls {
	int referenceCount;
	virtual void destroy(int);
	virtual void open(int);
	virtual void close(int);
	virtual void m3(int);
	virtual void m4();
	virtual void m5();
	virtual void m6();
	virtual void m7();
	virtual void m8();
	virtual void m9();
	virtual void m10();
	virtual void m11();
	virtual void m12();
	virtual void m13();
	virtual void m14();
	virtual void m15();
	virtual void setResult(int);
	virtual void m17();
	virtual void setTaggedWideText(int);
	virtual void m19();
	virtual void setTextArgument(int, int);
	virtual void m21();
	virtual void m22(int, int);
};
extern "C" {
// Start the requested BP script once loaded and apply all queued group visibility overrides.
void CInGameFadeDialog_StartScriptAndSetGroups(void* self) {
	// Phase 1: wait for an active dialog, then resolve its loaded BP object.
	char* item;
	if (!((RefCountedDialogCalls*)self)->isActive()) return;
	if (AsCInGameFadeDialog(self)->scriptStarted) return;
	if (Sing_805DC5F0() == 0) {
		if (!s_bpCtrlInitialized) {
			__ct__7CBpCtrlFv(s_bpCtrl);
			__register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
			s_bpCtrlInitialized = 1;
		}
		Sing_805DC5F0() = s_bpCtrl;
	}
	void* node = IdMap_Lookup(Sing_805DC5F0(), AsCInGameFadeDialog(self)->bpEntryId);
	if (node) {
		if (BpObject_PollLoad(node)) {
			// Phase 2: start once, apply name-based overrides, then latch completion.
			BpObject_StartScriptByName(node, !(AsCInGameFadeDialog(self)->scriptNameStorageFlags >> 31) ? ((FadeDialogScriptView*)self)->scriptName.inPlace.text : (*(char**)&AsCInGameFadeDialog(self)->scriptNameHeap), 1);
			for (item =(*(char**)&AsCInGameFadeDialog(self)->groupVisibilityOverrides); item != (*(char**)&AsCInGameFadeDialog(self)->groupVisibilityOverrides) + AsCInGameFadeDialog(self)->groupVisibilityOverrideCount * 16; item += 16) {
				void* target = BpObject_GetOrCreateNamedGroup(node, !(((DialogVisibilityOverride*)item)->groupName.heap.flags >> 31) ? ((DialogVisibilityOverride*)item)->groupName.inPlace.text : ((DialogVisibilityOverride*)item)->groupName.heap.data);
				if (target) {
					UIGroup_SetEnabled(target, ((DialogVisibilityOverride*)item)->enabled);
					if (((DialogVisibilityOverride*)item)->enabled) UIGroup_SetOpacity(target, kDialogFullyOpaque);
				}
			}
			AsCInGameFadeDialog(self)->scriptStarted = 1;
		}
	}
}

// Create the small dialog with layout kind 5, then register and return an owned ref.
// singleButtonMode is an encoded Boolean ABI word, NOT a BP script/path: the
// constructor stores it at CSmallCommonDialogLayout+4C and input handling selects
// one-button vs choice controls. Caller CGolfRule passes 0. taggedWideText is null
// or NUL-terminated UTF-16 tagged text (golfRuleMessageTags at the caller); its old
// const char* ABI spelling did not describe the encoding. Slot 18 is
// CSmallCommonDialog_SetExpandedWideText (small_common_dialog.cpp): expand synchronously with the dialog's
// owned textExpansionContext, copy wide text, then free expansion scratch. The
// input buffer is borrowed only for this call; no callback/context is retained.
// out required, unchanged on failure; success writes one caller reference, separate
// from the registry's reference. Release: decrement leading int refcount and call
// virtual destroy(1) at vtable+8 only on zero (as GolfTips_Release... does).
// Do not raw-delete or remove the registry's reference to discharge the caller's.
int SmallCommonDialog_CreateAndRegisterMode5(void* singleButtonMode, const unsigned short* taggedWideText, void** out) {
	if (out == 0) return 0;
	void* object = 0;
	if (!CSmallCommonDialog_Create((const char*)singleButtonMode, &object, kSmallDialogMode5)) return 0;
	((DialogTimingView*)object)->fadeSeconds = kFloat_0_2_CommonDialogFadeSeconds;
	((DialogTimingView*)object)->holdSeconds = kFloat_0_CommonDialog;
	((CommonDialogCalls*)object)->setResult(0);
	((CommonDialogCalls*)object)->setTaggedWideText((int)taggedWideText);
	((CommonDialogCalls*)object)->open(1);
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_AddUnique(Sing_805DC930(), object);
	if (out != 0) {
		void* ptr = object;
		*out = ptr;
		++*(int*)ptr;
	}
	void* ptr = object;
	int count = --*(int*)ptr;
	if (count == 0 && ptr != 0) {
		((CommonDialogCalls*)ptr)->destroy(1);
	}
	return 1;
}

// Draw the dialog BP object and aligned text at the current fade opacity; disable it at zero opacity.
void CInGameFadeDialog_Draw(void* self) {
	if (!((FadeDialogCalls*)self)->isReady()) return;
	int opacity128 = (int)(kFloat_128_DialogBpOpacity * AsCInGameFadeDialog(self)->fadeOpacity);
	if (opacity128 > 0) {
		if (Sing_805DC5F0() == 0) {
			if (!s_bpCtrlInitialized) {
				__ct__7CBpCtrlFv(s_bpCtrl);
				__register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
				s_bpCtrlInitialized = 1;
			}
			Sing_805DC5F0() = s_bpCtrl;
		}
		void* bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCInGameFadeDialog(self)->bpEntryId);
		if (bpObject) {
			BpObject_SetEnabled(bpObject, 1);
			BpCtrl_SetOpacity(AsCInGameFadeDialog(self)->bpEntryId, opacity128);
			BpObject_TryInvokeCurrentCallback(bpObject, 0);
			BpObject_Draw(bpObject);
		}
		if (!InGameDialog_GetNonemptyWideText(self)) return;
		void* bpObjectForText = bpObject;
		const char* textGroupName = !((unsigned int)AsCInGameFadeDialog(self)->textGroupNameStorageFlags >> 31) ? (const char*)&AsCInGameFadeDialog(self)->textGroupNameStorageFlags + 1 : (*(char**)&AsCInGameFadeDialog(self)->textGroupNameHeap);
		void* group = BpObject_GetOrCreateNamedGroup(bpObjectForText, textGroupName);
		if (!group) return;
		float textPos[2];
		UIGroup_GetPosition(group, textPos);
		DialogText_DrawAligned(self, textPos, Dialog_GetOpacityByte(self), 0, 0);
	} else {
		if (Sing_805DC5F0() == 0) {
			if (!s_bpCtrlInitialized) {
				__ct__7CBpCtrlFv(s_bpCtrl);
				__register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
				s_bpCtrlInitialized = 1;
			}
			Sing_805DC5F0() = s_bpCtrl;
		}
		void* bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCInGameFadeDialog(self)->bpEntryId);
		if (bpObject) BpObject_SetEnabled(bpObject, 0);
	}
}

// Clear the shared dialog registry nodes and reset the tips/tutorial resource pointers.
void GolfTips_ResetSharedRegistryAndResources() {
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_ClearNodes(Sing_805DC930());
	g_golfTipsDialog = 0;
	g_golfTipsMessageResource = 0;
	g_tutorialMessageResource = 0;
}

// Release the shared tips dialog and message resource through their reference counts.
void GolfTips_ReleaseDialogAndMessageResource() {
	void* ptr = g_golfTipsDialog;
	if (ptr != 0) {
		if (--*(int*)ptr == 0 && ptr != 0) {
			((RefCountedDialogCalls*)ptr)->destroy(1);
		}
		g_golfTipsDialog = 0;
	}
	ptr = g_golfTipsMessageResource;
	if (ptr != 0) {
		if (--*(int*)ptr == 0 && ptr != 0) {
			((RefCountedDialogCalls*)ptr)->destroy(1);
		}
		g_golfTipsMessageResource = 0;
	}
}

// Disable the dialog's BP object before running base-dialog close cleanup.
void CInGameFadeDialog_CompleteClose(void* self) {
	if (Sing_805DC5F0() == 0) {
		if (s_bpCtrlInitialized == 0) {
			char* p = s_bpCtrl;
			__ct__7CBpCtrlFv(p);
			__register_global_object(p, CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
			s_bpCtrlInitialized = 1;
		}
		Sing_805DC5F0() = s_bpCtrl;
	}
	void* ptr = IdMap_Lookup(Sing_805DC5F0(), AsCInGameFadeDialog(self)->bpEntryId);
	if (ptr != 0)
		BpObject_SetEnabled(ptr, 0);
	CInGameDialog_CompleteClose(self);
}

// Create the shared tips dialog, choose its text group/timing, and load the message resource.
int GolfTips_CreateDialogResources() {
	if (g_golfTipsDialog == 0 && CInGameFadeDialog_Create(golfTipsResourceStrings, &g_golfTipsDialog)) {
		if (g_golfTipsDialog == 0)
			return 0;
		InGameFadeDialog_SetTextGroupName(g_golfTipsDialog, golfTipsResourceStrings + kTipsLineGroupOffset);
		double half = kFloat_0_5_GolfTipsFadeSeconds;
		((DialogTimingView*)g_golfTipsDialog)->fadeSeconds = half;
		((DialogTimingView*)g_golfTipsDialog)->holdSeconds = kFloat_2_GolfTipsHoldSeconds;
		((DialogTimingView*)g_golfTipsDialog)->blockRegistryClose = 1;
		if (CPangyaMsgResourceObject_Create(0, &g_golfTipsMessageResource))
			return g_golfTipsMessageResource != 0;
	}
	return 0;
}

// Return a reference to the minimum comparison length.
inline const unsigned int& MinMessageLength(const unsigned int& a, const unsigned int& b) {
	return a < b ? a : b;
}
#pragma push
#pragma use_lmw_stmw on
// Suppress duplicate or transition-blocked tips, expand optional text arguments, and register the dialog.
// Requires index in [0,15): existing invalid-index paths pass a null name onward.
// Return 0 for missing resources, duplicate active script or pending transition;
// return 1 when opened. taggedMessageText is nullable NUL-terminated UTF-16,
// borrowed only during Text_ExpandTaggedText (mw_string_stream_and_text_tags.cpp). It is source text,
// not a context object or an integer message ID. Output uses the dialog's
// initialized 12-byte engine WString at stringStorageFlags; it must not alias
// source across growth. Ordinary code units copy (CR is dropped); backslash-<
// starts V=/L=/D= tags, with a required closing > and D= key <=255 code units.
// Unknown tags remain literal; replacements are not recursively expanded.
// flag 0 keeps named-lookup policy in the expander's default path. It can allocate
// and throw; this function has already removed/replaced the old script then, so
// it promises no transactional rollback. Null text skips expansion after the
// script-name setter has cleared old arguments; BP-only title scripts allow this.
// On success output, if nonnull, receives an added reference the caller releases;
// on failure output is untouched. holdSeconds/fadeSeconds are dialog timer values.
// Release that ONE output reference by decrementing the object's leading signed
// count and invoking RefCountedDialogCalls::destroy(1), its virtual deleting destructor, only
// when it reaches zero; see GolfTips_ReleaseDialogAndMessageResource's identical
// decrement/destroy idiom in this unit. This API supplies no standalone wrapper
// for releasing a caller's copy. CommonDialogRegistry_Remove drops only the REGISTRY reference;
// it does not discharge the output owner's reference. Do not call the global
// release routine to release a caller's copy, or directly delete a shared dialog.
// +38=1 opts shared tips out of bulk eligible-close sweeps. Explicit script/round
// close requests and positive holdSeconds still close normally; this flag does
// not force the dialog to remain open indefinitely or change text expansion.
int GolfTips_ShowIndexedMessage(int index, const unsigned short* taggedMessageText, void** output, float holdSeconds, float fadeSeconds) {
	// Phase 1: require both resources and reject the currently active script.
	void* dialog = g_golfTipsDialog;
	if (!dialog) return 0;
	if (!dialog) return 0;
	if (!g_golfTipsMessageResource) return 0;
	int scriptAddress;
	if (index < 0) scriptAddress = 0;
	else if (index < 0 || (unsigned int)index >= kTipScriptCount) scriptAddress = 0;
	else scriptAddress = ((int*)golfTipsDialogNames)[index];
	if (((RefCountedDialogCalls*)dialog)->isActive()) {
		unsigned int savedRequestedNameLength;
		dialog = g_golfTipsDialog;
		const char* name = (const char*)scriptAddress;
		unsigned int scriptNameLength;
		const unsigned int requestedNameLength = savedRequestedNameLength = strlen(name);
		if ((((FadeDialogScriptView*)g_golfTipsDialog)->scriptName.heap.flags >> 31) == 0) {
			scriptNameLength = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f;
		} else {
			scriptNameLength = ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
		}
		unsigned int storedNameLength;
		const char* storedScriptName;
		if (!(((FadeDialogScriptView*)dialog)->scriptName.heap.flags >> 31)) {
			storedScriptName = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.text;
			storedNameLength = ((FadeDialogScriptView*)dialog)->scriptName.inPlace.length & 0x7f;
		} else {
			storedScriptName = ((FadeDialogScriptView*)dialog)->scriptName.heap.data;
			storedNameLength = ((FadeDialogScriptView*)dialog)->scriptName.heap.length;
		}
		const unsigned int storedNameLengthCopy = storedNameLength;
		unsigned int minimumScriptLength = MinMessageLength(scriptNameLength, storedNameLengthCopy);
		int cmp = memcmp(storedScriptName, name, MinMessageLength(requestedNameLength, minimumScriptLength));
		if (cmp == 0) {
			if (minimumScriptLength < savedRequestedNameLength) cmp = -1;
			else if (minimumScriptLength == savedRequestedNameLength) cmp = cmp;
			else cmp = 1;
		}
		if (cmp == 0) return 0;
	}
	// Phase 2: stop/unregister the old script before replacing name and arguments.
	if (g_taskManager->transitionPending) return 0;
	((CommonDialogCalls*)g_golfTipsDialog)->close(0);
	if (!Sing_805DC930()) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_Remove(Sing_805DC930(), g_golfTipsDialog);
	GolfTipsDialog_SetScriptNameAndClearArguments(g_golfTipsDialog, (void*)scriptAddress);
	GolfTips_ConfigureDrivingContestTipGroups(index);
	// Phase 3: set timing, expand optional text, then open and retain in registry.
	((DialogTimingView*)g_golfTipsDialog)->fadeSeconds = fadeSeconds;
	((DialogTimingView*)g_golfTipsDialog)->holdSeconds = holdSeconds;
	((DialogTimingView*)g_golfTipsDialog)->blockRegistryClose = 1;
	InGameDialog_SetWideText(g_golfTipsDialog, 0);
	if (AsCPangyaMsgResourceObject(g_golfTipsMessageResource)->messageDraw && taggedMessageText)
		Text_ExpandTaggedText(AsCPangyaMsgResourceObject(g_golfTipsMessageResource)->messageDraw, (char*)&((CInGameDialog*)g_golfTipsDialog)->stringStorageFlags, (void*)taggedMessageText, 0);
	((CommonDialogCalls*)g_golfTipsDialog)->open(1);
	if (!Sing_805DC930()) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_AddUnique(Sing_805DC930(), g_golfTipsDialog);
	if (output) { void* ptr = g_golfTipsDialog; *output = ptr; ++*(int*)ptr; }
	return 1;
}
#pragma pop

// Close a matching tip when round flags block it, then update the registered dialogs by elapsed time.
// dtSeconds is the simulation step forwarded by CGolfTask_OnProcess
// (golf_task_loading_and_pause.cpp), using the CMainFrame seconds hook (main_frame_letter_and_mouse_cursor.cpp), not wall-clock
// milliseconds or the absolute g_currentTimeMilliseconds. Contract: finite,
// nonnegative delta; zero polls without advancing fades. No upper bound or clamp
// is enforced; a large delta may cross one state boundary per update and discard
// excess time. Negative/NaN deltas are not supported. Registry calls each dialog's
// update slot 9, polls completion slot 15, releases its ref and erases completed
// nodes (ingame_dialog_and_fade_dialog.cpp). Caller-held references remain valid. Serialized UI use only.
void GolfTips_UpdateRegisteredDialogs(float dtSeconds) {
	
	unsigned int mask = kBlockingRoundStates | ((SharedDoc_IsPlayerFlag0Set(AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)) ? 0 : kNonLocalPlayerBlockingState);
	if (*(unsigned int*)&AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags & mask)
		GolfTipsDialog_RequestClose(-2, kFloat_0_CommonDialog);
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_UpdateAndReleaseCompleted(Sing_805DC930(), dtSeconds);
}

// Destroy queued visibility-override strings and release their vector storage.
void* DialogOverrideVector_dtor(void* self, int shouldDelete) {
	if (self != 0) {
		char* end;
		char* first = ((DialogOverrideVector*)self)->data + ((DialogOverrideVector*)self)->firstIndex * 16;
		end = first + ((DialogOverrideVector*)self)->count * 16;
		while (end > first) {
			end -= 16;
			if (end != 0 && end != 0 && ((DialogVisibilityOverride*)end)->groupName.heap.flags >> 31)
				appFreeUni__FPv((void*)((DialogVisibilityOverride*)end)->groupName.heap.data);
		}
		((DialogOverrideVector*)self)->count = 0;
		if (self != 0) {
			int count;
			char* ptr = ((DialogOverrideVector*)self)->data;
			if (ptr != 0) {
			count = ((DialogOverrideVector*)self)->count;
			((DialogOverrideVector*)self)->count = 0;
			while (count != 0) {
				ptr -= 16;
				if (ptr != 0 && ptr != 0 && ((DialogVisibilityOverride*)ptr)->groupName.heap.flags >> 31)
					appFreeUni__FPv((void*)((DialogVisibilityOverride*)ptr)->groupName.heap.data);
				--count;
			}
			appFreeUni__FPv((void*)((DialogOverrideVector*)self)->data);
			}
		}
	if (shouldDelete > 0)
		__dl__FPv(self);
	}
	return self;
}

// 0x80258BCC: b TutorialDialog_PollSharedLayout; the tutorial state loop polls the result (0x802079E8).
// Forward shared tutorial-layout readiness to its caller.
int CTutorial_PollSharedDialogLayout() {
	return TutorialDialog_PollSharedLayout();
}

// Create/register a tutorial dialog and retain it for output; preserve validation before publication.
// pageKindWord is an encoded integer page/layout kind, NOT a CTutorial receiver
// or retained factory pointer. CInGameTutorialDialog_Ctor (ingame_tutorial_dialog.cpp) stores its
// integer value as tutorialIndex, clamps outside [0,7) to 0, and indexes
// s_tutorialPageDimensions plus the 7x32 visibility/cursor masks. Callers in
// tutorial_mode_ctutorial.cpp/tutorial_commands_and_input.cpp/tutorial_command_queue.cpp pass kinds 0,1,5,6. There is no context lifetime
// to extend: the factory consumes the numeric word synchronously. The shared
// tutorial layout/font resources must outlive the resulting registered dialogs.
// output is required and written only on
// success. Returns 0 without writing output for unready layout, null output,
// missing resource or failed creation. Out-of-range index still opens a dialog
// with its cleared text; it is not a creation failure. The registry and caller
// retain their own references; caller eventually decrements/destroys its reference.
// Fade is 0.2 seconds, hold=0 disables automatic timeout; message index is zero-based.
int TutorialDialog_CreateForMessageIndex(void* pageKindWord, int index, void** output) {
	if (!TutorialDialog_PollSharedLayout())
		return 0;
	if (!output)
		return 0;
	if (!g_tutorialMessageResource)
		return 0;
	void* object = 0;
	if (!CInGameTutorialDialog_Create(pageKindWord, &object))
		return 0;
	((DialogTimingView*)object)->fadeSeconds = kFloat_0_2_CommonDialogFadeSeconds;
	((DialogTimingView*)object)->holdSeconds = kFloat_0_CommonDialog;
	((CommonDialogCalls*)object)->setResult(0);
	((CommonDialogCalls*)object)->setTextArgument(0, 0);
	((CommonDialogCalls*)object)->setTextArgument(2, 0);
	InGameDialog_SetWideText(object, 0);
	void* list = AsCPangyaMsgResourceObject(g_tutorialMessageResource)->messageDraw;
	if (list != 0 && index >= 0 && index < MessageContext_GetResourceData(list))
		MessageTable_ExpandEntryText(list, (char*)&((CInGameDialog*)object)->stringStorageFlags, index, 0);
	((CommonDialogCalls*)object)->open(1);
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_AddUnique(Sing_805DC930(), object);
	if (output) {
		*output = object;
		++*(int*)object;
	}
	if (object != 0) {
		if (--*(int*)object == 0) {
			((CommonDialogCalls*)object)->destroy(1);
		}
	}
	return 1;
}

// Start shared tutorial-layout loading and create tutorial_msg.ecb; preserve separate readiness polling.
// Returns whether g_tutorialMessageResource was created, not whether layout/text
// loading has finished. CTutorial_PollSharedDialogLayout is the readiness query.
// The global resource is retained until TutorialDialog_ReleaseSharedResources.
int Tutorial_LoadMessageResources() {
	TutorialLayout_LoadSharedResource();
	CPangyaMsgResourceObject_Create(golfTipsResourceStrings + kTutorialMessagesOffset, &g_tutorialMessageResource);
	return g_tutorialMessageResource != 0;
}

// Release the shared tutorial layout and reference-counted message resource.
void TutorialDialog_ReleaseSharedResources() {
	TutorialLayout_DestroySharedResource();
	void* ptr = g_tutorialMessageResource;
	if (ptr != 0) {
		if (--*(int*)ptr == 0 && ptr != 0) {
			((RefCountedDialogCalls*)ptr)->destroy(1);
		}
		g_tutorialMessageResource = 0;
	}
}

// Destroy the party-item base and optionally free the fire-item object.
void* party_CPartyItemFireBantage_dtor(void* ptr, int shouldDelete) {
	if (ptr != 0) {
		party_IItem_dtor(ptr, 0);
		if (shouldDelete > 0)
			__dl__FPv(ptr);
	}
	return ptr;
}

// Report that the fade-dialog interface is available.
int CInGameFadeDialog_vfunc_8() {
	return 1;
}

// Initialize Fire Bantage's inactive party-item modifier defaults (borrow arg
// under PartyIItem_ctor's owner protocol; return self, no modifier applied yet).
// +14=0.3 is a DIMENSIONLESS fractional club-distance gain, zero neutral. The
// ApplyOwnerModifiers/ApplyModifiersIfActive hooks (party_item_gameplay_hooks.cpp) copy it into
// PartyItemModifiers+00. GolfParameter_ResolveFloat (club_selection_and_shot_params.cpp) consumes it in
// party mode 3 as FMADDS(baseDistanceYards,rate,baseDistanceYards): baseline 1.3x,
// excluding selected putter and inventory slot-14 special club set. Increasing
// the rate increases that parameter's distance; this is not a world-unit offset
// or a direct velocity multiplier. Safe retuning limits are not established.
// +18=-0.8 is the impact half-width fractional gain (1+gain), copied to modifier
// +04 and consumed by the club/impact code, leaving baseline 0.2x width. Keep
// activation/player filtering in the hooks; changing defaults does not enable it.
void* CPartyItemFireBantage_Ctor(void* self, void* arg) {
	PartyIItem_ctor(self, arg, 0, 0);
	float second;
	float first = kFloat_0_3_FireBantage;
	void* vtable = __vt__Q25party21CPartyItemFireBantage;
	second = kFloat_Neg0_8_FireBantage;
	// Offset zero stores the item vtable, not a dialog override vector.
	*(void**)self = vtable;
	AsCPartyItemFireBantage(self)->active = 0;
	// +0x14 is the club-distance bonus rate; this constructor installs its default.
	AsCPartyItemFireBantage(self)->clubDistanceBonusRate = first;
	AsCPartyItemFireBantage(self)->impactZoneWidthBonus = second;
	return self;
}

// Ensure the common-dialog registry exists, then release all registered object references.
void RefObjectList_ReleaseAllSharedObjects() {
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	RefObjectList_ReleaseEntries(Sing_805DC930());
}

// Ask registered dialogs without either close-suppression flag to process their close callback.
void CommonDialogRegistry_CloseEligibleDialogs() {
	if (Sing_805DC930() == 0) {
		if (!s_refObjectListInitialized) {
			RefObjectList_Init(s_refObjectList);
			__register_global_object(s_refObjectList, RefObjectList_dtor, s_refObjectListDtorRegistration_PartyItem);
			s_refObjectListInitialized = 1;
		}
		Sing_805DC930() = s_refObjectList;
	}
	CommonDialogRegistry_RequestEligibleClose(Sing_805DC930());
}

// Return whether the BP script has been started.
void* CInGameFadeDialog_vfunc_14(void* arg) {
	return (*(void**)&AsCInGameFadeDialog(arg)->scriptStarted);
}

// Clear script-start state so a new dialog script can be loaded.
int CInGameFadeDialog_ResetScriptStarted(void* arg) {
	AsCInGameFadeDialog(arg)->scriptStarted = 0;
	return 1;
}

// Dispatch the derived fade-dialog drawing callback.
void CInGameFadeDialog_vfunc_4(void* arg) {
	((FadeDialogCalls*)arg)->drawFade();
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void GolfTips_InitTauConstant() {
	s_golfTipsTau = kFloat_6_2831855;
}

// Store the small common dialog's result.
void CSmallCommonDialog_vfunc_16(void* arg, int value) {
	AsCSmallCommonDialog(arg)->result = value;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void FireBantage_InitTauConstant() {
	s_fireBantageTau = kFloat_6_2831855;
}

// Return the shared tips dialog's active state, or zero when no dialog exists.
int GolfTips_IsDialogActive() {
	void* ptr = g_golfTipsDialog;
	if (ptr == 0)
		return 0;
	if (ptr != 0)
		return ((TipReadinessCalls*)ptr)->isActive();
	return 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622F8", _ctors_u802579FC_0, GolfTips_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804622F8", _ctors_u802579FC_1, FireBantage_InitTauConstant);
#pragma pop
