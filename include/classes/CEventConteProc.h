#ifndef CLASSES_CEVENTCONTEPROC_H
#define CLASSES_CEVENTCONTEPROC_H

#include "game/types.h"

struct Fn80094B44Arg;
struct Fn80094EB8Arg;
struct Fn8009911CArg;
struct Fn80099188Arg;
struct Fn80099288Arg;
struct Fn80099348Arg;
struct Fn800993A4Arg;
struct Fn8009940CArg;
struct Fn80099468Arg;
struct Fn800994C4Arg;
struct Fn80099508Arg;
struct Fn80099A88Arg;
struct ScriptCommandContextView;
struct Fn8009BTargetArg;
struct Fn8009B654Arg;
struct Fn8009B794Arg;
struct Fn8009B8A0Arg;
struct Fn8009BDFCArg;
class CEventConteProc {
public:
    void* vtbl;           // 0x0: original vtable pointer
    char pad_004[0x4];    // 0x4
    char eventNameTag;    // 0x8
    char inlineEventName; // 0x9
    char pad_00A[0xa];    // 0xa
    char eventKindTag;    // 0x14
    char inlineEventKind; // 0x15

    int ProcessExitCommand();
    int SkipCommand4();
    int SkipCommand8();
    int SkipCommand12();
    int SkipCommand16();
    int SkipCommand12_C();
    int ProcessIfCommand();
    int ProcessElseIfCommand();
    int ProcessElseCommand();
    int ProcessEndIfCommand();
    int SkipCommand8_B();
    int ProcessGotoCommand();
    int SkipThreeStrings();
    int SkipTwoStrings();
    int SkipOneString();
    int Advance8();
    int Advance8_B();
    int Advance4();
    int ProcessCameraSetCommand();
    int ProcessThenCommand();
    int ProcessWaitCommand();
    int WaitForModelSlotChange();

    int ProcessModelFaceCommand();
    int ProcessModelHeadCommand();
    int ProcessModelHeadResetCommand();
    int ProcessModelMotionCommand();
    int ProcessModelMoveCommand();
    int ProcessModelRotationCommand();
    static int ProcessFacePartsCommand(Fn80094B44Arg* script);
    static int ExecuteFaceSubscript(void* script);
    int ProcessFaceStopCommand();
    static int ProcessFaceTimeCommand(Fn80094EB8Arg* script);
    int ProcessEffectFadeCommand();
    int ExecuteAttachedEffectCommand();
    int ExecuteMessageRequestCommand();
    int ProcessMessageWindowCommand();
    int ProcessSoundSeRequestCommand();
    int ProcessSoundSePlayCommand();
    int ProcessSoundSeStopCommand();
    int ProcessSoundSePitchCommand();
    int ProcessSoundBgmPlayCommand();
    int ProcessSoundBgmStopCommand();
    int ProcessSoundBgmVolumeCommand();
    int ProcessSoundBgmPitchCommand();
    static int ProcessSetFlagCommand(Fn8009911CArg* script);
    static int ProcessBindFlagVariableCommand(Fn80099188Arg* script);
    static int ProcessBindParameterVariableCommand(Fn80099188Arg* script);
    static int ProcessVariableJumpCommand(Fn80099288Arg* script);
    static int ProcessWaitCameraCommand(Fn80099348Arg* script);
    static int ProcessWaitModelCommand(Fn800993A4Arg* script);
    static int ProcessWaitEffectCommand(Fn8009940CArg* script);
    static int ProcessWaitMessageCommand(Fn80099468Arg* script);
    static int ProcessSetStartPointCommand(Fn800994C4Arg* script);
    static int ProcessBlockPointCommand(Fn80099508Arg* script);
    int ProcessSealPointCommand();
    static int ProcessMoveCharacterCommand(Fn80099A88Arg* script);
    int ProcessObjectDrawCommand();
    static int ProcessEventSkipCommand(ScriptCommandContextView* script);
    static int ProcessIsYesNoDialogCommand(ScriptCommandContextView* script);
    static int ProcessIsTourFlagCommand(Fn8009BTargetArg* script);
    static int ProcessIsCharacterCommand(Fn8009B654Arg* script);
    static int ProcessIsCaddieCommand(Fn8009BTargetArg* script);
    static int ProcessIsCostumeCommand(Fn8009B794Arg* script);
    static int ProcessIsClubSetCommand(Fn8009B794Arg* script);
    static int ProcessIsCoinNumCommand(Fn8009B8A0Arg* script);
    static int ProcessIsBlockCommand(Fn8009BTargetArg* script);
    static int ProcessSetAppearRoadInfoCommand(Fn8009BTargetArg* script);
    static int ProcessAppearRoadCommand(Fn8009BTargetArg* script);
    static int ProcessIsTourClearCommand(void* script);
    static int ProcessConteSkipCommand(Fn8009BDFCArg* script);

    ~CEventConteProc();
};

#define CEVENTCONTEPROC_OFF(f, o) typedef char CEventConteProc_off_##f[((int)&((CEventConteProc*)0)->f == (o)) ? 1 : -1]
CEVENTCONTEPROC_OFF(eventNameTag, 0x8);
CEVENTCONTEPROC_OFF(inlineEventName, 0x9);
CEVENTCONTEPROC_OFF(eventKindTag, 0x14);
CEVENTCONTEPROC_OFF(inlineEventKind, 0x15);
#undef CEVENTCONTEPROC_OFF

#endif // CLASSES_CEVENTCONTEPROC_H
