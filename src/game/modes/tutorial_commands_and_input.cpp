namespace named_fields {
// Tutorial callbacks use the same layout as the adjacent CTutorial methods.
#include "classes/CTutorial.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include "classes/GameInputManager.h"
#include "classes/CMouseCursor.h"
#include "classes/CInGameTutorialDialog.h"
}
// View the existing storage as named_fields::CTutorial without transferring ownership.
static inline named_fields::CTutorial* AsCTutorial(const volatile void* p) { return (named_fields::CTutorial*)p; }
// View the existing storage as named_fields::CGolfDoc without transferring ownership.
static inline named_fields::CGolfDoc* AsCGolfDoc(const volatile void* p) { return (named_fields::CGolfDoc*)p; }
// View the existing storage as named_fields::CSharedDoc without transferring ownership.
static inline named_fields::CSharedDoc* AsCSharedDoc(const volatile void* p) { return (named_fields::CSharedDoc*)p; }
// View the existing storage as named_fields::CMouseCursor without transferring ownership.
static inline named_fields::CMouseCursor* AsCMouseCursor(const volatile void* p) { return (named_fields::CMouseCursor*)p; }
// View the existing storage as named_fields::CInGameTutorialDialog without transferring ownership.
static inline named_fields::CInGameTutorialDialog* AsTutorialDialog(const volatile void* p) { return (named_fields::CInGameTutorialDialog*)p; }
// View the existing storage as named_fields::GameInputManager without transferring ownership.
static inline named_fields::GameInputManager* AsGameInputManager(const volatile void* p) { return (named_fields::GameInputManager*)p; }
// 0x8020907C / 0x80208B30: controller lookup gate +0x154, tutorial state +0x3C,
// and owned tutorial callback objects +0x5C/+0x60 use their existing layouts.
// Translation unit 0x80208B30-0x80209C3C (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Existing byte pool: extents from globals6; Shift-JIS text stays in its original storage.
struct TutorialCommandStrings {
    char screenActor[7]; // 0x0
    char scriptFilename[13]; // 0x7
    char integerFormat[3]; // 0x14
    char cameraTargetModel[18]; // 0x17
    char cameraActor[7]; // 0x29
    char startPoint[7]; // 0x30
    char message100[4]; // 0x37
    char screenMessageFormat[10]; // 0x3B
    char message101[4]; // 0x45
    char landingPoint1_1[12]; // 0x49
    char caddieNamedPointFormat[15]; // 0x55
    char caddieMessageFormat[13]; // 0x64
    char caddie1_1[14]; // 0x71
    char message102[4]; // 0x7F
    char landingPoint1_2[12]; // 0x83
    char caddie1_2[14]; // 0x8F
    char message103[4]; // 0x9D
    char landingPoint1_3[12]; // 0xA1
    char caddie1_3[14]; // 0xAD
    char message104[4]; // 0xBB
    char landingPoint1_4[12]; // 0xBF
    char caddie1_4[14]; // 0xCB
    char message105[4]; // 0xD9
    char landingPoint1_5[12]; // 0xDD
    char caddie1_5[14]; // 0xE9
    char message106[4]; // 0xF7
    char landingPoint1_6[12]; // 0xFB
    char caddie1_6[14]; // 0x107
    char message107[4]; // 0x115
    char landingPoint3[10]; // 0x119
    char bunkerStartPoint[16]; // 0x123
    char landingPoint4[10]; // 0x133
    char greenStartPoint[16]; // 0x13D
    char dialog7RectFormat[15]; // 0x14D
    char enabledArgument[2]; // 0x15C
    char dialog55RectFormat[16]; // 0x15E
    char dialog80RectFormat[16]; // 0x16E
    char dialog90RectFormat[17]; // 0x17E
    char dialog400x120Format[11]; // 0x18F
    char dialog384x184Format[14]; // 0x19A
    char golfRuleValueFormat[15]; // 0x1A8
    char golfRuleActor[9]; // 0x1B7
};
enum TutorialStringOffset {
    kTutorialString_screenActor = (int)&((TutorialCommandStrings*)0)->screenActor,
    kTutorialString_scriptFilename = (int)&((TutorialCommandStrings*)0)->scriptFilename,
    kTutorialString_integerFormat = (int)&((TutorialCommandStrings*)0)->integerFormat,
    kTutorialString_cameraTargetModel = (int)&((TutorialCommandStrings*)0)->cameraTargetModel,
    kTutorialString_cameraActor = (int)&((TutorialCommandStrings*)0)->cameraActor,
    kTutorialString_startPoint = (int)&((TutorialCommandStrings*)0)->startPoint,
    kTutorialString_message100 = (int)&((TutorialCommandStrings*)0)->message100,
    kTutorialString_screenMessageFormat = (int)&((TutorialCommandStrings*)0)->screenMessageFormat,
    kTutorialString_message101 = (int)&((TutorialCommandStrings*)0)->message101,
    kTutorialString_landingPoint1_1 = (int)&((TutorialCommandStrings*)0)->landingPoint1_1,
    kTutorialString_caddieNamedPointFormat = (int)&((TutorialCommandStrings*)0)->caddieNamedPointFormat,
    kTutorialString_caddieMessageFormat = (int)&((TutorialCommandStrings*)0)->caddieMessageFormat,
    kTutorialString_caddie1_1 = (int)&((TutorialCommandStrings*)0)->caddie1_1,
    kTutorialString_message102 = (int)&((TutorialCommandStrings*)0)->message102,
    kTutorialString_landingPoint1_2 = (int)&((TutorialCommandStrings*)0)->landingPoint1_2,
    kTutorialString_caddie1_2 = (int)&((TutorialCommandStrings*)0)->caddie1_2,
    kTutorialString_message103 = (int)&((TutorialCommandStrings*)0)->message103,
    kTutorialString_landingPoint1_3 = (int)&((TutorialCommandStrings*)0)->landingPoint1_3,
    kTutorialString_caddie1_3 = (int)&((TutorialCommandStrings*)0)->caddie1_3,
    kTutorialString_message104 = (int)&((TutorialCommandStrings*)0)->message104,
    kTutorialString_landingPoint1_4 = (int)&((TutorialCommandStrings*)0)->landingPoint1_4,
    kTutorialString_caddie1_4 = (int)&((TutorialCommandStrings*)0)->caddie1_4,
    kTutorialString_message105 = (int)&((TutorialCommandStrings*)0)->message105,
    kTutorialString_landingPoint1_5 = (int)&((TutorialCommandStrings*)0)->landingPoint1_5,
    kTutorialString_caddie1_5 = (int)&((TutorialCommandStrings*)0)->caddie1_5,
    kTutorialString_message106 = (int)&((TutorialCommandStrings*)0)->message106,
    kTutorialString_landingPoint1_6 = (int)&((TutorialCommandStrings*)0)->landingPoint1_6,
    kTutorialString_caddie1_6 = (int)&((TutorialCommandStrings*)0)->caddie1_6,
    kTutorialString_message107 = (int)&((TutorialCommandStrings*)0)->message107,
    kTutorialString_landingPoint3 = (int)&((TutorialCommandStrings*)0)->landingPoint3,
    kTutorialString_bunkerStartPoint = (int)&((TutorialCommandStrings*)0)->bunkerStartPoint,
    kTutorialString_landingPoint4 = (int)&((TutorialCommandStrings*)0)->landingPoint4,
    kTutorialString_greenStartPoint = (int)&((TutorialCommandStrings*)0)->greenStartPoint,
    kTutorialString_dialog7RectFormat = (int)&((TutorialCommandStrings*)0)->dialog7RectFormat,
    kTutorialString_enabledArgument = (int)&((TutorialCommandStrings*)0)->enabledArgument,
    kTutorialString_dialog55RectFormat = (int)&((TutorialCommandStrings*)0)->dialog55RectFormat,
    kTutorialString_dialog80RectFormat = (int)&((TutorialCommandStrings*)0)->dialog80RectFormat,
    kTutorialString_dialog90RectFormat = (int)&((TutorialCommandStrings*)0)->dialog90RectFormat,
    kTutorialString_dialog400x120Format = (int)&((TutorialCommandStrings*)0)->dialog400x120Format,
    kTutorialString_dialog384x184Format = (int)&((TutorialCommandStrings*)0)->dialog384x184Format,
    kTutorialString_golfRuleValueFormat = (int)&((TutorialCommandStrings*)0)->golfRuleValueFormat,
    kTutorialString_golfRuleActor = (int)&((TutorialCommandStrings*)0)->golfRuleActor,
};
// Queue call contract: (tutorial, opcode, payload, delayMilliseconds, repeatCount).
// -1 waits for an event; 0 runs without a timed delay. Payload is copied by the queue:
// opcodes 1/2/4 copy one action byte; other opcodes copy bounded text (see tutorial_command_queue.cpp).
// Stage dispatch below: 0 intro/aim points, 1 stage-1 builder, 2 swing/mode-2 builder,
// 3 bunker points, 4 green/mode-3 builder, 5 final scripted messages.
enum TutorialCommandOpcode {
    kCommandWait = 0, kCommandPulseActionAlternate = 4,
    kCommandStageEndDialog = 6, kCommandMessageDialog = 7,
    kCommandMessageLayout2 = 8, kCommandMessageLayout0 = 10,
    kCommandSizedMessageDialog = 12, kCommandTargetNamedPoint = 13,
    kCommandDispatchActorMessage = 14, kCommandBlockInput = 16,
    kCommandCloseDialog = 20
};
enum { kCommandsPerBlock = 25, kRetainedCommandBlocks = 2, kEmptyCommandOffset = 12,
       kTutorialRunningCommands = 5, kTutorialCompletionDialogState = 3,
       kTutorialActionCount = 39, kPrimaryCompletionAction = 19,
       kSecondaryCompletionAction = 24, kCompletionCursorAction = 17 };
enum TutorialFieldOffset {
    kTutorialStageFlagsOffset = (int)&((named_fields::CTutorial*)0)->stage0Completed,
    kTutorialCallbackOffset = (int)&((named_fields::CTutorial*)0)->secondaryVtable,
    kTutorialMessageDialogOffset = (int)&((named_fields::CTutorial*)0)->messageDialog,
    kTutorialModeDialogOffset = (int)&((named_fields::CTutorial*)0)->modeMessageDialog,
    kInputActionsOffset = (int)&((named_fields::GameInputManager*)0)->actionBlocked
};
// Only tutorial-reset fields of the round entry are known in this unit.
struct TutorialPlayerRoundView {
    char pad_000[0x28];
    unsigned char holeFlags[18];
    char pad_03A[0x74];
    unsigned char tutorialResetByte;
    char pad_0AF[0x1D];
    int tutorialResetWord;
};
struct TutorialNamedPoint { char nameAndMetadata[0x24]; unsigned int positionWords[3]; };
typedef char TutorialRound_wordOffset[((int)&((TutorialPlayerRoundView*)0)->tutorialResetWord == 0xCC) ? 1 : -1];

#include "game/singletons.h"
extern "C" {

extern signed char s_inputManagerInitialized;
extern void *g_mouseCursor;
extern named_fields::CSharedDoc *g_sharedDoc;
extern void *g_inputPointer;
extern char tutorialCommandStrings[];
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_tutorial[];
extern char s_tutorialLessonCompletionCallback[];
extern char s_tutorialModeCompletionCallback[];
extern int zeroInitializationWords[];
extern float kFloat_0_5_Tutorial;
extern float kFloat_119_0;
extern float kFloat_82_InGameDialog;
extern void InputManager_Init(void *);
extern void InputManager_Dtor();
extern void __register_global_object(void *, void (*)(), void *);
extern void Pointer_ClearInputSuppression(void *);
extern void Pointer_SuppressInput(void *);
extern void IActor_DispatchMessage(void *, void *, const char *, int, int, int, int, int);
extern void TutorialDialog_CreateForMessageIndex(int, int, void *, float);
extern void *Runtime_AdjustMemberPointer(int, const char *, void *);
extern void appFreeUni__FPv(void *);
extern void *GolfPlayer_GetEntryByIndex(void *, int);
extern void *CourseData_FindNamedPoint(void *, const char *);
extern int String_FormatTemporary(const char *, ...);
extern void strcpy(char *, const char *);
extern void CTutorial_AppendRepeatedCommands(void *, int, const char *, int, int);
extern void CTutorial_BuildStage1Commands(void *);
extern void CTutorial_BuildStage2Mode2Commands(void *);
extern void CTutorial_BuildSwingingPracticeCommands(void *);
extern void CTutorial_BuildLandingPoint4Commands(void *);
extern void CTutorial_BuildStage4Mode3Commands(void *);
extern void CTutorial_BuildStage4DefaultCommands(void *);

// Clear queued commands, reset the player, build the selected lesson and block input for its opening dialog.
// self and player 0 must exist; named course points must resolve before formatting their XYZ.
// Rebuilding owns/frees command blocks, but course points and singleton state are borrowed.
void CTutorial_ResetLessonCommands(void *self, int stage)
{
    // One word shares non-overlapping reset, hole-index and pool-address lifetimes.
    int lessonValue;
    char startPointName[32];
    char *s = (char *)self;
    int emptyReadOffset;

    // Phase 1: drain records and release consumed 25-command blocks. Keep two blocks
    // of read-offset slack; the empty logical origin is 12, not zero.
    if (AsCTutorial(s)->pendingCommandCount) {
        lessonValue = 0;
        emptyReadOffset = kEmptyCommandOffset;
        do {
            --AsCTutorial(s)->pendingCommandCount;
            ++AsCTutorial(s)->commandReadOffset;
            while (*(unsigned int *)&AsCTutorial(s)->commandReadOffset >= kCommandsPerBlock * kRetainedCommandBlocks) {
                appFreeUni__FPv(((void**)AsCTutorial(s)->commandBlocks)[*(unsigned int *)&AsCTutorial(s)->firstCommandBlockIndex]);
                if (++*(unsigned int *)&AsCTutorial(s)->firstCommandBlockIndex == AsCTutorial(s)->commandBlockCapacity) AsCTutorial(s)->firstCommandBlockIndex = lessonValue;
                --AsCTutorial(s)->commandBlockCount;
                AsCTutorial(s)->commandReadOffset -= kCommandsPerBlock;
            }
            if (!AsCTutorial(s)->pendingCommandCount) AsCTutorial(s)->commandReadOffset = emptyReadOffset;
        } while (AsCTutorial(s)->pendingCommandCount);
    }
    // Phase 2: reset lesson/player flags before choosing the stage's spawn and command script.
    AsCTutorial(s)->state = kTutorialRunningCommands;
    emptyReadOffset = 0;
    AsCTutorial(s)->stepCountdown = -1;
    AsCTutorial(s)->forcedActionIndex = -1;
    char optionalLandingPointName[32];
    startPointName[0] = 0;
    optionalLandingPointName[0] = 0;
    void *st = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
    ((TutorialPlayerRoundView*)st)->tutorialResetWord = 0;
    st = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
    ((TutorialPlayerRoundView*)st)->tutorialResetByte = 0;
    lessonValue = (unsigned char)*(char*)&AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
    void *p = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
    ((TutorialPlayerRoundView*)p)->holeFlags[lessonValue] = 1;
    st = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(st)->tutorialMarkerVisible = 0;
    st = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    AsCGolfDoc(st)->tutorialStage = stage;
    int selectedStage = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
    // Phase 3: append commands in execution order; formatted temporary text is copied on append.
    switch (selectedStage) {
    case 0: {
        AsCTutorial(s)->aimAlignmentCount = 0;
        lessonValue = (int)tutorialCommandStrings;
        strcpy(startPointName, (char *)(lessonValue + kTutorialString_startPoint));
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message100), -1, 1);

        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)(String_FormatTemporary((char *)(lessonValue + kTutorialString_screenMessageFormat), 0x1E)), 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message101), -1, 1);
        void *lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        void *namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_1));
        int formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_1));
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_1), 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandStageEndDialog, (char *)0, -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message102), -1, 1);
        lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_2));
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_2));
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_2), -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message103), -1, 1);
        lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_3)); formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_3));
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_3), -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message104), -1, 1);
        lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_4));
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_4));
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_4), -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message105), -1, 1);
        lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_5));
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_5));
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_5), -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message106), -1, 1);
        lessonDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        namedPoint = CourseData_FindNamedPoint(AsCGolfDoc(lessonDoc)->courseData, (char *)(lessonValue + kTutorialString_landingPoint1_6));
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieNamedPointFormat), 4, ((TutorialNamedPoint*)namedPoint)->positionWords);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_caddieMessageFormat), 2, (char *)(lessonValue + kTutorialString_caddie1_6));

        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1); CTutorial_AppendRepeatedCommands(self, kCommandTargetNamedPoint, (char *)(lessonValue + kTutorialString_landingPoint1_6), -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)(lessonValue + kTutorialString_message107), -1, 1);
    } break;
    case 1: {
        // Both point names come from the string pool; lessonValue no longer holds the hole index.
        lessonValue = (int)tutorialCommandStrings;
        strcpy(startPointName, (char *)(lessonValue + kTutorialString_startPoint));
        strcpy(optionalLandingPointName, (char *)(lessonValue + kTutorialString_landingPoint3));
        CTutorial_BuildStage1Commands(self);
    } break;
    case 2: {
        lessonValue = (int)tutorialCommandStrings;     // original 80209664-80209680
        strcpy(startPointName, (char *)(lessonValue + kTutorialString_startPoint));
        strcpy(optionalLandingPointName, (char *)(lessonValue + kTutorialString_landingPoint3));
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 2) CTutorial_BuildStage2Mode2Commands(self); else CTutorial_BuildSwingingPracticeCommands(self);
    } break;
    case 3: {
        lessonValue = (int)tutorialCommandStrings;     // original 802096AC-802096C8
        strcpy(startPointName, (char *)(lessonValue + kTutorialString_bunkerStartPoint));
        strcpy(optionalLandingPointName, (char *)(lessonValue + kTutorialString_landingPoint4));
        CTutorial_BuildLandingPoint4Commands(self);
    } break;
    case 4: {
        strcpy(startPointName, tutorialCommandStrings + kTutorialString_greenStartPoint);
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 3) CTutorial_BuildStage4Mode3Commands(self); else CTutorial_BuildStage4DefaultCommands(self);
    } break;
    case 5: {
        lessonValue = (int)tutorialCommandStrings;
        strcpy(startPointName, (char *)(lessonValue + kTutorialString_startPoint));
        char completionAction = 6;
        CTutorial_AppendRepeatedCommands(self, kCommandPulseActionAlternate, &completionAction, 0, 1);
        int formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_integerFormat), 0x34);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_screenMessageFormat), 0x20);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog7RectFormat), 0x35);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageLayout0, (char *)formattedCommand, -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandBlockInput, (char *)(lessonValue + kTutorialString_enabledArgument), 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_screenMessageFormat), 0x1F);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_screenMessageFormat), 0x1E);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog55RectFormat), 0x36);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageLayout0, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog80RectFormat), 0x37);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageLayout0, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog80RectFormat), 0x38);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageLayout0, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog90RectFormat), 0x39);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageLayout2, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_integerFormat), 0x3A);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog400x120Format), 0x3B);
        CTutorial_AppendRepeatedCommands(self, kCommandStageEndDialog, (char *)formattedCommand, -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandCloseDialog, (char *)0, 0, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_integerFormat), 0x3C);
        CTutorial_AppendRepeatedCommands(self, kCommandMessageDialog, (char *)formattedCommand, -1, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_dialog384x184Format), 0x64, 4);
        CTutorial_AppendRepeatedCommands(self, kCommandSizedMessageDialog, (char *)formattedCommand, -1, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandBlockInput, (char *)(lessonValue + kTutorialString_enabledArgument), 0, 1);
        CTutorial_AppendRepeatedCommands(self, kCommandWait, (char *)0, 0x1F4, 1);
        formattedCommand = String_FormatTemporary((char *)(lessonValue + kTutorialString_golfRuleValueFormat), 0x12, AsCTutorial(s)->completedStageCount);
        CTutorial_AppendRepeatedCommands(self, kCommandDispatchActorMessage, (char *)formattedCommand, -1, 1);
    } break;
    }

    // Phase 4: block all actions except 19, suppress the pointer, and dispatch spawn setup.
    if (!Sing_805DC5C8()) { if (!s_inputManagerInitialized) {
        InputManager_Init(s_gameInputManager);
        __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_tutorial);
        s_inputManagerInitialized = 1;
    }
        Sing_805DC5C8() = s_gameInputManager;
    }
    void *inputManager = Sing_805DC5C8();
    for (int actionIndex = 0; actionIndex < kTutorialActionCount; ++actionIndex) AsGameInputManager(inputManager)->actionBlocked[actionIndex] = 1;
    if (!inputManager) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_tutorial);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    inputManager = Sing_805DC5C8();
    AsGameInputManager(inputManager)->selectedPadLookupDisabled = 1;
    if (inputManager == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_tutorial);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    inputManager = Sing_805DC5C8();
    AsGameInputManager(inputManager)->actionBlocked[kPrimaryCompletionAction] = 0;
    Pointer_SuppressInput(g_inputPointer);
    void *golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(golfDoc)->tutorialMarkerVisible = 0;
    ((void (*)(void *, void *, const char *, int, int, int, int, int))IActor_DispatchMessage)(self, self, (char *)((char*)&tutorialCommandStrings), 0x1C, 0, 0, 0, 0);
    ((void (*)(void *, void *, const char *, int, int, int, int, int))IActor_DispatchMessage)(self, self, (char *)((char*)&tutorialCommandStrings), 0x1D, 0, 0, 0, 0);

    // Convert an empty optional point name to a null argument without changing the original mask arithmetic.
    int firstOptionalCharacter = (signed char)((unsigned char)optionalLandingPointName[0]);
    int negativeFirstCharacter = -firstOptionalCharacter;
    // These value aliases retain the original operand order selected by CodeWarrior.
    int combinedSignBits = firstOptionalCharacter | negativeFirstCharacter;
    int signBitsAlias = combinedSignBits;
    int retainedSignBits = signBitsAlias;
    int optionalNameMask = (negativeFirstCharacter | firstOptionalCharacter) >> 31;

    ((void (*)(void *, void *, const char *, int, int, int, int, int))IActor_DispatchMessage)(self, self, (char *)((char*)&tutorialCommandStrings + kTutorialString_golfRuleActor), 0x0E, (int)startPointName, ((int)optionalLandingPointName & optionalNameMask), 0, 0);
}

// Construct and publish the input singleton without re-registering an existing instance.
static inline void InitializeTutorialInput(void)
{
    if (s_inputManagerInitialized == 0) {
        InputManager_Init(s_gameInputManager);
        __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_tutorial);
        s_inputManagerInitialized = 1;
    }
    Sing_805DC5C8() = s_gameInputManager;
}

// Ensure the input manager exists before changing tutorial action gates.
static inline void *GetTutorialInput(void)
{
    if (Sing_805DC5C8() == 0) {
        InitializeTutorialInput();
    }
    return Sing_805DC5C8();
}

struct Arr3 { int v[3]; };

class TutorialDialogInterface {
public:
    int ref;
    virtual void destroy(int);
    virtual void v1();
    // Virtual slot 2 is called with 0 during dialog teardown; its exact contract is unresolved.
    virtual void formattedCommand(int);
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void setState(int);
    virtual void setCallback(void *, void *);
    virtual void v18();
    virtual void v19();
};

typedef named_fields::GameInputManager TutorialInputState;

// Mark first-time completion, restore selected actions and attach the lesson-specific completion dialog.
// Idempotent while modeMessageDialog exists. Other dialog paths may run again; only
// firstStageCompletion gates recording the completed stage. Dialog references are owned.
void CTutorial_BeginStageCompletion(void *self)
{
    unsigned char *tutorialBytes = (unsigned char *)self;
    Arr3 emptyCallback;
    Arr3 lessonCallback;
    Arr3 lessonCallbackScratch;
    Arr3 modeCallback;
    Arr3 modeCallbackScratch;
    TutorialInputState *inputState;
    void *dialog;
    void *callbackTarget;

    if (AsCTutorial(tutorialBytes)->modeMessageDialog != 0)
        return;
    // Phase 1: record a first completion before restoring action gates.
    if (*(unsigned char*)&AsCTutorial(tutorialBytes)->firstStageCompletion) {
        unsigned char *golfDoc = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        int stageIndex = AsCGolfDoc(golfDoc)->tutorialStage;
        *((unsigned char*)&AsCTutorial(tutorialBytes)->stage0Completed + stageIndex) = 1;
    }
    inputState = (TutorialInputState *)GetTutorialInput();
    inputState->actionBlocked[0] = 1; inputState->actionBlocked[1] = 1; inputState->actionBlocked[2] = 1; inputState->actionBlocked[3] = 1; inputState->actionBlocked[4] = 1;
    inputState->actionBlocked[5] = 1; inputState->actionBlocked[6] = 1; inputState->actionBlocked[7] = 1; inputState->actionBlocked[8] = 1; inputState->actionBlocked[9] = 1;
    inputState->actionBlocked[10] = 1; inputState->actionBlocked[11] = 1; inputState->actionBlocked[12] = 1; inputState->actionBlocked[13] = 1; inputState->actionBlocked[14] = 1;
    inputState->actionBlocked[15] = 1; inputState->actionBlocked[16] = 1; inputState->actionBlocked[17] = 1; inputState->actionBlocked[18] = 1; inputState->actionBlocked[19] = 1;
    inputState->actionBlocked[20] = 1; inputState->actionBlocked[21] = 1; inputState->actionBlocked[22] = 1; inputState->actionBlocked[23] = 1; inputState->actionBlocked[24] = 1;
    inputState->actionBlocked[25] = 1; inputState->actionBlocked[26] = 1; inputState->actionBlocked[27] = 1; inputState->actionBlocked[28] = 1; inputState->actionBlocked[29] = 1;
    inputState->actionBlocked[30] = 1; inputState->actionBlocked[31] = 1; inputState->actionBlocked[32] = 1; inputState->actionBlocked[33] = 1; inputState->actionBlocked[34] = 1;
    inputState->actionBlocked[35] = 1; inputState->actionBlocked[36] = 1; inputState->actionBlocked[37] = 1; inputState->actionBlocked[38] = 1;
    if (inputState == 0)
        InitializeTutorialInput();
    inputState = (TutorialInputState *)Sing_805DC5C8();
    AsGameInputManager(inputState)->selectedPadLookupDisabled = 1;
    if (inputState == 0)
        InitializeTutorialInput();
    inputState = (TutorialInputState *)Sing_805DC5C8();
    inputState->actionBlocked[kPrimaryCompletionAction] = 0;
    if (inputState == 0)
        InitializeTutorialInput();
    inputState = (TutorialInputState *)Sing_805DC5C8();
    inputState->actionBlocked[kSecondaryCompletionAction] = 0;
    if (inputState == 0)
        InitializeTutorialInput();
    inputState = (TutorialInputState *)Sing_805DC5C8();
    inputState->actionBlocked[kCompletionCursorAction] = 0;
    // Phase 2: pointer/action gates are restored; detach the old callback before releasing its dialog.
    Pointer_ClearInputSuppression(g_inputPointer);
    if (AsCTutorial(tutorialBytes)->messageDialog) {
        callbackTarget = self;
        if (self)
            callbackTarget = (char*)self + kTutorialCallbackOffset;
        emptyCallback = *(Arr3 *)zeroInitializationWords;
        ((TutorialDialogInterface *)AsCTutorial(tutorialBytes)->messageDialog)->setCallback(&emptyCallback, callbackTarget);
        ((TutorialDialogInterface *)AsCTutorial(tutorialBytes)->messageDialog)->setState(2);
        ((TutorialDialogInterface *)AsCTutorial(tutorialBytes)->messageDialog)->formattedCommand(0);
        dialog = AsCTutorial(tutorialBytes)->messageDialog;
        if (dialog != 0) {
            if (--((TutorialDialogInterface *)dialog)->ref == 0) {
                if (dialog != 0)
                    ((TutorialDialogInterface *)dialog)->destroy(1);
            }
            AsCTutorial(tutorialBytes)->messageDialog = 0;
        }
    }
    // Phase 3: notify screen/camera, create the stage-specific dialog, and attach its callback.
    // Member callbacks target the secondary interface at +4; Runtime_AdjustMemberPointer
    // compensates the receiver so the member function still sees the complete tutorial object.
    AsCTutorial(tutorialBytes)->state = kTutorialCompletionDialogState;
    IActor_DispatchMessage(self, self, tutorialCommandStrings, 0x53, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, tutorialCommandStrings + kTutorialString_cameraActor, 0x0B, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, tutorialCommandStrings, 0x51, 0, 0, 0, 0);
    IActor_DispatchMessage(self, self, tutorialCommandStrings, 0x50, 0, 0, 0, 0);
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == 2) {
        TutorialDialog_CreateForMessageIndex(1, (AsCSharedDoc(g_sharedDoc)->gameMode == 2 ? 0x11 : 0x27), tutorialBytes + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
        callbackTarget = self;
        if (self)
            callbackTarget = (char*)self + kTutorialCallbackOffset;
        Arr3 *adjustedLessonCallback = (Arr3 *)Runtime_AdjustMemberPointer(-kTutorialCallbackOffset, s_tutorialLessonCompletionCallback, &lessonCallbackScratch);
        lessonCallback = *adjustedLessonCallback;
        dialog = AsCTutorial(tutorialBytes)->messageDialog;
        ((TutorialDialogInterface *)dialog)->setCallback(&lessonCallback, callbackTarget);
        dialog = AsCTutorial(tutorialBytes)->messageDialog;
        {
            float f0;
            float f1;
            f1 = kFloat_119_0;
            f0 = kFloat_82_InGameDialog;
            AsTutorialDialog(dialog)->positionX = f1;
            AsTutorialDialog(dialog)->positionY = f0;
        }
    } else {
        TutorialDialog_CreateForMessageIndex(0, 8, tutorialBytes + kTutorialModeDialogOffset, kFloat_0_5_Tutorial);
        callbackTarget = self;
        if (self)
            callbackTarget = (char*)self + kTutorialCallbackOffset;
        Arr3 *adjustedModeCallback = (Arr3 *)Runtime_AdjustMemberPointer(-kTutorialCallbackOffset, s_tutorialModeCompletionCallback, &modeCallbackScratch);
        modeCallback = *adjustedModeCallback;
        dialog = AsCTutorial(tutorialBytes)->modeMessageDialog;
        ((TutorialDialogInterface *)dialog)->setCallback(&modeCallback, callbackTarget);
        dialog = AsCTutorial(tutorialBytes)->modeMessageDialog;
        {
            float f0;
            float f1;
            f1 = kFloat_119_0;
            f0 = kFloat_82_InGameDialog;
            AsTutorialDialog(dialog)->positionX = f1;
            AsTutorialDialog(dialog)->positionY = f0;
        }
        AsCMouseCursor(g_mouseCursor)->moveToActive = 0;
    }
}

} // extern "C"
