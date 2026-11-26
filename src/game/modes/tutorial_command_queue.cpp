// Tutorial command queue: the ring of tutorial command blocks, its iterators, and the executor that runs
// each lesson command (dialogs, input blocking, cursor moves, save flags). Also builds the lesson sequences.

#include <decomp/static_init.h>
extern "C" char s_tutorialDialogRectMemberDescriptor[];
#include "MSL/deque.h"
// Packed byte view of the existing string pool; storage and encoding stay in the original symbol.
struct TutorialCommandStrings {
    char screenActor[7]; // 0x0: "Screen"
    char reserved007[0xD];
    char integerFormat[3]; // 0x14: "%d"
    char reserved017[0x12];
    char cameraActor[7]; // 0x29: "Camera"
    char reserved030[0xB];
    char screenMessageFormat[10]; // 0x3B: "Screen %d"
    char reserved045[0x10];
    char caddieNamedPointFormat[15]; // 0x55: "Caddie %d %d 1"
    char caddieMessageFormat[13];    // 0x64: "Caddie %d %d"
    char reserved071[0xA8];
    char landingPoint3[10]; // 0x119: "\u843d\u4e0b\u652f\u5e973"
    char reserved123[0x10];
    char landingPoint4[10]; // 0x133: "\u843d\u4e0b\u652f\u5e974"
    char reserved13D[0x1F];
    char enabledArgument[2]; // 0x15C: "1"
    char reserved15E[0x3C];
    char dialog384x184Format[14];         // 0x19A: "%d 384 184 %d"
    char golfRuleValueFormat[15];         // 0x1A8: "GolfRule %d %d"
    char golfRuleActor[9];                // 0x1B7: "GolfRule"
    char cursorPositionFormat[6];         // 0x1C0: "%f %f"
    char dialogIndexPositionFormat[9];    // 0x1C6: "%d %f %f"
    char dialogIndexRectFormat[15];       // 0x1CF: "%d %f %f %f %f"
    char dialogIndexSizeKindFormat[12];   // 0x1DE: "%d %f %f %d"
    char actorMessageFormat[15];          // 0x1EA: "%s %d %d %d %d"
    char effectActor[7];                  // 0x1F9: "Effect"
    char screenValueFormat[13];           // 0x200: "Screen %d %d"
    char cameraValueFormat[13];           // 0x20D: "Camera %d %d"
    char screenDisableFormat[12];         // 0x21A: "Screen %d 0"
    char dialogSwingRectFormat[18];       // 0x226: "%d -95 27 112 275"
    char pointer350x250[8];               // 0x238: "350 250"
    char pointer70x120[7];                // 0x240: "70 120"
    char pointer70x190[7];                // 0x247: "70 190"
    char dialog400x50Format[10];          // 0x24E: "%d 400 50"
    char dialogShotRectFormat[17];        // 0x258: "%d 0 128 207 376"
    char dialogLeftShotRectFormat[19];    // 0x269: "%d -29 128 139 376"
    char golfRuleEnabledValuesFormat[20]; // 0x27C: "GolfRule %d 1 %d %d"
    char golfRuleEnableFormat[14];        // 0x290: "GolfRule %d 1"
    char dialogRightShotRectFormat[19];   // 0x29E: "%d 104 128 475 376"
    char golfRuleDisableFormat[14];       // 0x2B1: "GolfRule %d 0"
    char disabledArgument[2];             // 0x2BF: "0"
    char dialog14x35RectFormat[17];       // 0x2C1: "%d 14 35 108 147"
    char golfRuleMessageFormat[12];       // 0x2D2: "GolfRule %d"
    char golfRuleValuesFormat[18];        // 0x2DE: "GolfRule %d %d %d"
    char dialogBunkerRectFormat[19];      // 0x2F0: "%d -103 128 65 376"
    char stage4CaddieName[14];            // 0x303: "\u30ad\u30e3\u30c7\u30a3\u30fc4_1"
    char dialog384x160Format[14];         // 0x311: "%d 384 160 %d"
    char gridEnableFormat[10];            // 0x31F: "Grid %d 1"
    char golfRuleEnabledValueFormat[17];  // 0x329: "GolfRule %d 1 %d"
    char gridDisableFormat[10];           // 0x33A: "Grid %d 0"
    char dequeLengthError[21];            // 0x344: "deque:: length error"
    char lengthErrorTypeName[55];         // 0x359: "!std::exception!!std::logic_error!!std::length_error!!"
    char circularDequeLengthError[20];    // 0x390: "cdeque length error"
};
enum TutorialStringOffsets {
    kTutorialString_screenActor = (int)&((TutorialCommandStrings*)0)->screenActor,
    kTutorialString_integerFormat = (int)&((TutorialCommandStrings*)0)->integerFormat,
    kTutorialString_cameraActor = (int)&((TutorialCommandStrings*)0)->cameraActor,
    kTutorialString_screenMessageFormat = (int)&((TutorialCommandStrings*)0)->screenMessageFormat,
    kTutorialString_caddieNamedPointFormat = (int)&((TutorialCommandStrings*)0)->caddieNamedPointFormat,
    kTutorialString_caddieMessageFormat = (int)&((TutorialCommandStrings*)0)->caddieMessageFormat,
    kTutorialString_landingPoint3 = (int)&((TutorialCommandStrings*)0)->landingPoint3,
    kTutorialString_landingPoint4 = (int)&((TutorialCommandStrings*)0)->landingPoint4,
    kTutorialString_enabledArgument = (int)&((TutorialCommandStrings*)0)->enabledArgument,
    kTutorialString_dialog384x184Format = (int)&((TutorialCommandStrings*)0)->dialog384x184Format,
    kTutorialString_golfRuleValueFormat = (int)&((TutorialCommandStrings*)0)->golfRuleValueFormat,
    kTutorialString_golfRuleActor = (int)&((TutorialCommandStrings*)0)->golfRuleActor,
    kTutorialString_cursorPositionFormat = (int)&((TutorialCommandStrings*)0)->cursorPositionFormat,
    kTutorialString_dialogIndexPositionFormat = (int)&((TutorialCommandStrings*)0)->dialogIndexPositionFormat,
    kTutorialString_dialogIndexRectFormat = (int)&((TutorialCommandStrings*)0)->dialogIndexRectFormat,
    kTutorialString_dialogIndexSizeKindFormat = (int)&((TutorialCommandStrings*)0)->dialogIndexSizeKindFormat,
    kTutorialString_actorMessageFormat = (int)&((TutorialCommandStrings*)0)->actorMessageFormat,
    kTutorialString_effectActor = (int)&((TutorialCommandStrings*)0)->effectActor,
    kTutorialString_screenValueFormat = (int)&((TutorialCommandStrings*)0)->screenValueFormat,
    kTutorialString_cameraValueFormat = (int)&((TutorialCommandStrings*)0)->cameraValueFormat,
    kTutorialString_screenDisableFormat = (int)&((TutorialCommandStrings*)0)->screenDisableFormat,
    kTutorialString_dialogSwingRectFormat = (int)&((TutorialCommandStrings*)0)->dialogSwingRectFormat,
    kTutorialString_pointer350x250 = (int)&((TutorialCommandStrings*)0)->pointer350x250,
    kTutorialString_pointer70x120 = (int)&((TutorialCommandStrings*)0)->pointer70x120,
    kTutorialString_pointer70x190 = (int)&((TutorialCommandStrings*)0)->pointer70x190,
    kTutorialString_dialog400x50Format = (int)&((TutorialCommandStrings*)0)->dialog400x50Format,
    kTutorialString_dialogShotRectFormat = (int)&((TutorialCommandStrings*)0)->dialogShotRectFormat,
    kTutorialString_dialogLeftShotRectFormat = (int)&((TutorialCommandStrings*)0)->dialogLeftShotRectFormat,
    kTutorialString_golfRuleEnabledValuesFormat = (int)&((TutorialCommandStrings*)0)->golfRuleEnabledValuesFormat,
    kTutorialString_golfRuleEnableFormat = (int)&((TutorialCommandStrings*)0)->golfRuleEnableFormat,
    kTutorialString_dialogRightShotRectFormat = (int)&((TutorialCommandStrings*)0)->dialogRightShotRectFormat,
    kTutorialString_golfRuleDisableFormat = (int)&((TutorialCommandStrings*)0)->golfRuleDisableFormat,
    kTutorialString_disabledArgument = (int)&((TutorialCommandStrings*)0)->disabledArgument,
    kTutorialString_dialog14x35RectFormat = (int)&((TutorialCommandStrings*)0)->dialog14x35RectFormat,
    kTutorialString_golfRuleMessageFormat = (int)&((TutorialCommandStrings*)0)->golfRuleMessageFormat,
    kTutorialString_golfRuleValuesFormat = (int)&((TutorialCommandStrings*)0)->golfRuleValuesFormat,
    kTutorialString_dialogBunkerRectFormat = (int)&((TutorialCommandStrings*)0)->dialogBunkerRectFormat,
    kTutorialString_stage4CaddieName = (int)&((TutorialCommandStrings*)0)->stage4CaddieName,
    kTutorialString_dialog384x160Format = (int)&((TutorialCommandStrings*)0)->dialog384x160Format,
    kTutorialString_gridEnableFormat = (int)&((TutorialCommandStrings*)0)->gridEnableFormat,
    kTutorialString_golfRuleEnabledValueFormat = (int)&((TutorialCommandStrings*)0)->golfRuleEnabledValueFormat,
    kTutorialString_gridDisableFormat = (int)&((TutorialCommandStrings*)0)->gridDisableFormat,
    kTutorialString_dequeLengthError = (int)&((TutorialCommandStrings*)0)->dequeLengthError,
    kTutorialString_lengthErrorTypeName = (int)&((TutorialCommandStrings*)0)->lengthErrorTypeName,
    kTutorialString_circularDequeLengthError = (int)&((TutorialCommandStrings*)0)->circularDequeLengthError,
};

#include "classes/CMouseCursor.h"
#include "classes/GameInputManager.h"
#include "classes/CInGameTutorialDialog.h"
#include "classes/WView.h"
#include "classes/CTutorial.h"
#define AsCTutorial(p) ((CTutorial*)(p))
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm exceptional cleanup of this object with its matching destructor.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm the unwind owner before explicit normal destruction.
    void release() {
        active = false;
    }
    // Destroy only when normal cleanup has not already taken responsibility.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

#include "game/singletons.h"
#include "game/wvector.h"
#include "sdk_protos.h"

extern "C" {

// The queue stores kCommandsPerBlock fixed-size command records per allocated block.
// Preserve the original maximum-count bit pattern used by the deque length guard.
static const int kCommandDequeMaxCountWord = -46;
enum TutorialQueueLimits {
    kCommandsPerBlock = 25,
    kCommandTextBytes = 32,
    kRetainedCommandBlocks = 2
};
// Word/byte views below derive offsets from headers; direct member pointers change MWCC register lifetimes.
enum TutorialWordOffsets {
    kTutorialWord_commandBlockCount = (int)&((CTutorial*)0)->commandBlockCount / sizeof(unsigned int),
    kTutorialWord_commandReadOffset = (int)&((CTutorial*)0)->commandReadOffset / sizeof(unsigned int),
    kTutorialWord_pendingCommandCount = (int)&((CTutorial*)0)->pendingCommandCount / sizeof(unsigned int),
    kTutorialWord_commandBlocks = (int)&((CTutorial*)0)->commandBlocks / sizeof(unsigned int),
};
static const int kTutorialMessageDialogOffset = (int)&((CTutorial*)0)->messageDialog;
static const int kTutorialPendingActionsOffset = (int)&((GameInputManager*)0)->pendingActionPress;
enum TutorialState {
    kTutorialWaitingForDialog = 0,
    kTutorialStageEnding = 1,
    kTutorialAimAlignment = 4,
    kTutorialRunningCommands = 5
};
enum TutorialCommandOpcode {
    kCommandWait = 0,
    kCommandPulseAction = 1,
    kCommandForceAction = 2,
    kCommandClearForcedAction = 3,
    kCommandPulseActionAlternate = 4,
    kCommandMoveCursor = 5,
    kCommandStageEndDialog = 6,
    kCommandMessageDialog = 7,
    kCommandMessageLayout2 = 8,
    kCommandMessageLayout4 = 9,
    kCommandMessageLayout0 = 10,
    kCommandMessageAtPointer = 11,
    kCommandSizedMessageDialog = 12,
    kCommandTargetNamedPoint = 13,
    kCommandDispatchActorMessage = 14,
    kCommandParseInteger = 15,
    kCommandBlockInput = 16,
    kCommandAllowInput = 17,
    kCommandLessonPrompt = 18,
    kCommandAimAlignment = 19,
    kCommandCloseDialog = 20,
    kCommandSetSaveFlag = 21,
    kCommandFinishCursorMove = 22,
};
// Hexadecimal opcode/payload reference for command producers and this unit's executor:
// 0x00 wait: none; 0x01/0x02/0x04 action: one signed byte (pulse/force/alternate pulse);
// 0x03 clear force: none; 0x05 cursor: "x y" in UI coordinates (delay is move duration);
// 0x06 stage-end: "message x y"; 0x07/0x0B message: "message" (fixed/pointer-relative);
// 0x08/0x09/0x0A layout: "message dx dy width height"; 0x0C size: "message width height kind";
// 0x0D target: named course point; 0x0E actor: "actor message value1 value2 value3";
// 0x0F integer: parsed but ignored; 0x10 block: integer also controls pointer suppression;
// 0x11 allow: none; 0x12 prompt/0x13 aim: integer enable; 0x14 close/0x15 save/0x16 stop cursor: none.
// Actor argument values may encode pointers as decimal integers; this is a 32-bit ABI.
// Payload text is at most 32 bytes; append copies it but does not ensure trailing NUL
// when the source fills the field. Producers must supply valid, parseable payloads.
// delayMilliseconds: 0 chains immediately, >0 arms the timer, -1 waits for an event.
// Append's pointer-typed delay argument carries the integer bits and is NEVER dereferenced.
// Every command is consumed after its side effects, including dialog creation; callbacks
// resume with the NEXT queued record. Payload storage is borrowed only during execution.
enum TutorialInputPolicyId {
    kTutorialScriptAction6 = 6,
    kTutorialScriptAction8 = 8,
    kTutorialDialogAction19 = 19,
    kTutorialDialogAction24 = 24,
    kTutorialDialogAction17 = 17,
    kTutorialBlockedAction20 = 20,
    kTutorialScriptAction26 = 26
};
// These are GameInputManager action-slot indices (0..38), not WPAD button masks.
// Script actions index pendingActionPress; policy IDs index actionBlocked.
// Physical button/gesture identities are unresolved in this unit. Policy matrix:
// waiting for dialog: block all, allow 17/19/24; pointer active, selected-pad lookup off.
// stage ending: allow all except 20; pointer and selected-pad lookup active.
// aim alignment: allow all except 6/20; pointer and selected-pad lookup active.
// Running commands retain the policy installed by the most recent opcode.
enum TutorialDelay {
    kTutorialWaitForEvent = -1
};
// Offsets into tutorialMessageIndices are 12-byte MW member-function-pointer
// records, not message-text IDs. The adjusted owner is tutorial.secondaryVtable;
// -0x18 translates that interface back to the CTutorial base. Keep all three
// callback words and the owner adjustment together when changing a dialog.
// Dialog continuation contract (payload units are UI pixels):
// opcode                  payload                       callback offset  state
// stage-end               index x y                     0x58 (*)         StageEnding
// message                 index                         0x64             WaitingForDialog
// layout2/4/0             index dx dy width height       0x70/7C/88       WaitingForDialog
// at-pointer              index                         0x94             WaitingForDialog
// sized-message           index width height kind       0xA0             WaitingForDialog
// lesson-prompt (enabled) integer                        0xAC             WaitingForDialog
// (*) Stage 1 begins completion directly; stage 5 installs a zero callback triple.
// Layout origins: plain message (139,82), layout variants (119+dx,82+dy),
// at-pointer (messageX-22-overflow,messageY-128), sized centered at y=82.
// Each callback triple is adjusted by -0x18 and registered against the secondary
// interface owner. Registration must copy its stack scratch before returning.
// The command is consumed AFTER registration; a result continues at the next
// record, not the dialog opcode. Parsing failures are unchecked legacy behavior.
enum TutorialCallbackOffset {
    kCallbackStageEnd = 0x58,
    kCallbackMessage = 0x64,
    kCallbackLayout2 = 0x70,
    kCallbackLayout4 = 0x7C,
    kCallbackLayout0 = 0x88,
    kCallbackAtPointer = 0x94,
    kCallbackSizedMessage = 0xA0,
    kCallbackLessonPrompt = 0xAC
};
// Receiver-verified IDs: CScreen_HandleMsg (ingame_hud_screen.cpp), CCamera::HandleMsg (camera_input_and_ground_view.cpp).
enum TutorialActorCommand {
    kScreenResetMapCameraAction = 0x45,
    kCameraSetZoomInputBlocked = 9,
    kCameraSetDirectionalInputEnabled = 11
};
// Lesson operands checked against CScreen_HandleMsg and CGolfRule::HandleMsg.
// GolfRule 0x0c enables auto-shot with integer UI x/y in its next two operands;
// 0x1f passes power/100 and its second operand to swing-controller slot 0x40.
// Slot-labelled operations retain neutral names until their complete contracts are known.
enum TutorialLessonActorMessage {
    kScreenShowTutorialMeter = 0x47,
    kScreenShowTutorialGauge = 0x49,
    kScreenShowTutorialAim = 0x4b,
    kScreenConfirmMapCameraAction = 0x46,
    kScreenSetTutorialClubChangeEnabled = 0x50,
    kScreenSetTutorialAimControlEnabled = 0x51,
    kScreenSetTutorialSpinControlEnabled = 0x52,
    kScreenLegacyMessage53 = 0x53, // No case in the current CScreen_HandleMsg reconstruction.
    kGolfRuleRunSwingController3C = 0x1e,
    kGolfRuleEnableSwingController4C = 0x20,
    kGolfRuleRunSwingController54 = 0x22,
    kGolfRuleSetAutoShot = 0xc,
    kGolfRuleSetProcessingPaused = 0xd,
    kGolfRuleConfigureAutoSwing = 0x1f,
};

enum TutorialProgressMessage {
    kScreenResetShotPanels = 0x0F,
    kScreenShowTutorialGaugeImmediately = 0x4A,
    kScreenShowTutorialAimImmediately = 0x4C,
    kGolfRuleRefreshBgmAndVoice = 0x16,
    kScreenSetTutorialCompletion = 0x1E,
    kScreenHideTutorialMeter = 0x4D,
    kScreenHideTutorialGauge = 0x4E,
    kScreenHideTutorialAim = 0x4F,
    kScreenSetCameraControlsEnabled = 0x55,
    kEffectLightColumnFadeIn = 0x0C // CParticleEffect_HandleMsg sets existing lightColumn.fadeMode=1.
};
enum {
    kTutorialEmptyReadOffset = 12,
    kTutorialCallbackThisAdjustment = -0x18
};
struct TutorialCommandView {
    unsigned int opcode;
    union {
        signed char action;
        char text[kCommandTextBytes];
    } payload;
    int delayMilliseconds;
};
// Deque header at CTutorial +1C: read offset, live command count, block-ring data,
// first block index, block count, ring capacity. Each block owns 25 records of 40 bytes.
// Usable command capacity excludes one block-ring sentinel slot; allocation/growth
// also reserves ring bookkeeping space. Consuming commands keeps a consumed prefix
// until readOffset >=50, then frees a block and subtracts 25; empty offset resets to12.
// Ring iterators carry current/allocation-begin/logical-origin/allocation-end pointers.
// The origin disambiguates wrap in distance/copy operations; it is not unused data.
// Single-wrap iterator helpers require offsets within one allocated ring span.
typedef std::deque<TutorialCommandView, app::app_allocator<TutorialCommandView> > TutorialCommandDeque;
typedef std::cdeque<void*, app::app_allocator<void*> > TutorialBlockRing;
struct TutorialQueueWords {
    unsigned int readOffset, commandCount, blocks, firstBlock, blockCount, blockCapacity;
};
typedef char TutorialCommandDeque_size[(sizeof(TutorialCommandDeque) == sizeof(TutorialQueueWords)) ? 1 : -1];
struct RingStorageWords {
    unsigned int data, start, used, capacity;
};
// The named-point API returns a byte record; +0x24 is its three-word position payload.
struct TutorialNamedPoint {
    char nameAndMetadata[0x24];
    unsigned int positionWords[3];
};
// SaveRecord_CopyCtor provides a complete 0x828-byte record; this command changes one persisted bit.
struct TutorialSaveRecord {
    char reserved[0xA0];
    unsigned int flags;
    char tail[0x784];
};
enum TutorialSaveFlags {
    kTutorialSaveCompletionFlag = 0x80
};
static const int kTutorialStageFlagsOffset = (int)&((CTutorial*)0)->stage0Completed;
typedef char TutorialCommandSizeCheck[sizeof(TutorialCommandView) == 40 ? 1 : -1];
typedef char TutorialSaveSizeCheck[sizeof(TutorialSaveRecord) == 0x828 ? 1 : -1];

extern void RingBuffer_ThrowLengthError(const char*);
extern unsigned int RingBuffer32_GetUsableCapacity(void*);
extern unsigned int RingBuffer32_GetCountWord(void*);
extern unsigned int RingBuffer_IsEmpty(void*);
extern void* UnsignedWord_SelectMaxPointer(unsigned int*, unsigned int*);
extern void RingBuffer32_ReallocateStorage(void*, void*);
extern void RingBuffer32_ResetCount(void*);
extern void* TutorialCommandDeque_InitializeEmptyBlockRing(void*, void*);
extern void* TutorialCommandDeque_GetReadOffsetWord(void*);
extern int TutorialCommandDeque_GetMaximumCountWord();
extern void* TutorialCommandBlock_AllocateRecords(int, int);
extern void* RingBuffer_GetBackPtr(void*);
extern void TutorialBlockRing_SwapStorageWords(void*, void*);
extern void TutorialBlockRing_MakeEndIterator(void*, void*);
extern void TutorialBlockRing_MakeBeginIterator(void*, void*);
extern void TutorialBlockRing_IteratorSubtractOffset(void*, const void*, int);
extern void RingBuffer32_InsertRangeWithIteratorCopies(void*, void*, void*, void*);
extern int TutorialCommandDeque_GetUsableCommandCapacity(void*);
extern void* TutorialCommandDeque_GetBlockRing(void*);
extern void* TutorialCommandDeque_GetBlockAllocatorView(void*);
extern void* RingBuffer_GetFrontPtr(void*);
extern void RingBuffer_PopFront(void*);
extern void TutorialBlockRing_AppendWord(void*, void*);
extern void* TutorialBlockRing_dtor(void*, int);

extern const char tutorialCommandStrings[];

struct LocalE54 {
    unsigned int data[6];
    // Construct an empty temporary command-block queue.
    LocalE54(void* iterator) {
        TutorialCommandDeque_InitializeEmptyBlockRing(this, iterator);
    }
    // Free only blocks/storage still owned by this temporary queue.
    ~LocalE54() {
        TutorialBlockRing_dtor(this, -1);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Recycle consumed blocks or grow the block ring to reserve space for additional tutorial commands.
void TutorialCommandDeque_ReserveBackBlocks(void* arg, int count) {
    unsigned int* remaining = (unsigned int*)arg;
    void* ring = TutorialCommandDeque_GetBlockRing(remaining);
    void* iterator = TutorialCommandDeque_GetBlockAllocatorView(remaining);
    unsigned int quotient = (unsigned int)count / kCommandsPerBlock;
    unsigned int rem = (unsigned int)count - quotient * kCommandsPerBlock;
    int whole = (int)(quotient + (rem != 0));
    unsigned int total = 0;
    // Phase 1: recycle fully consumed front blocks before allocating any new storage.
    while (whole != 0 && (unsigned int)TutorialCommandDeque_GetReadOffsetWord(remaining) >= kCommandsPerBlock) {
        unsigned int value = *(unsigned int*)RingBuffer_GetFrontPtr(ring);
        RingBuffer_PopFront(ring);
        *(unsigned int*)RingBuffer_GetBackPtr(ring) = value;
        unsigned int zero = total;
        TutorialBlockRing_AppendWord(ring, &zero);
        --whole;
        *remaining -= kCommandsPerBlock;
    }
    // Phase 2: validate command capacity, then append blocks or grow the pointer ring.
    if (whole != 0) {
        total = (unsigned int)whole * kCommandsPerBlock;
        if (total > (unsigned int)((int (*)(void*))TutorialCommandDeque_GetMaximumCountWord)(remaining) ||
            (total = (unsigned int)((int (*)(void*))TutorialCommandDeque_GetMaximumCountWord)(remaining)-total,
             (unsigned int)TutorialCommandDeque_GetUsableCommandCapacity(remaining) > total)) {
            RingBuffer_ThrowLengthError((tutorialCommandStrings + kTutorialString_dequeLengthError));
        }
        unsigned int first = (unsigned int)RingBuffer32_GetUsableCapacity(ring);
        unsigned int size = (unsigned int)RingBuffer32_GetCountWord(ring);
        if ((unsigned int)whole + size <= first) {
            while (whole != 0) {
                void* item = TutorialCommandBlock_AllocateRecords((int)iterator, kCommandsPerBlock);
                *(void**)RingBuffer_GetBackPtr(ring) = item;
                unsigned int zero = 0;
                TutorialBlockRing_AppendWord(ring, &zero);
                --whole;
            }
        } else {
            unsigned int v2[4];
            unsigned int v1[4];
            unsigned int c[4];
            unsigned int a[4];
            LocalE54 temp(iterator);
            void* work = TutorialCommandDeque_GetBlockRing(&temp);
            unsigned int cap;
            unsigned int end = (unsigned int)RingBuffer32_GetCountWord(ring) + (unsigned int)whole + 1;
            cap = (unsigned int)RingBuffer32_GetUsableCapacity(ring) * 2;
            void* returned = UnsignedWord_SelectMaxPointer(&cap, &end);
            RingBuffer32_ReallocateStorage(work, (void*)*(unsigned int*)returned);
            unsigned int zero = 0;
            TutorialBlockRing_AppendWord(work, &zero);
            while (whole != 0) {
                void* item = TutorialCommandBlock_AllocateRecords((int)iterator, kCommandsPerBlock);
                *(void**)RingBuffer_GetBackPtr(work) = item;
                unsigned int zero = 0;
                TutorialBlockRing_AppendWord(work, &zero);
                --whole;
            }
            if (RingBuffer_IsEmpty(ring) == 0) {
                TutorialBlockRing_MakeEndIterator(a, ring);
                TutorialBlockRing_IteratorSubtractOffset(c, a, 1);
                TutorialBlockRing_MakeBeginIterator(v1, ring);
                TutorialBlockRing_MakeBeginIterator(v2, work);
                ((void (*)(void*, void*, void*, void*, int))RingBuffer32_InsertRangeWithIteratorCopies)(work, v2, v1, c,
                                                                                                        0);
            }
            // Commit the new ring; reset its temporary predecessor so live blocks are not freed.
            TutorialBlockRing_SwapStorageWords(ring, work);
            RingBuffer32_ResetCount(work);
        }
    }
}
#pragma pop

struct RingPointerIteratorWithFlag {
    void** cur;
    void** base;
    void** end;
    void** limit;
    bool flag;
};
struct RingPointerIterator {
    void** cur;
    void** base;
    void** end;
    void** limit;
};
// One pointer-ring model backs command block storage. Four-word cursors describe
// position/base/logical-origin/limit; the five-word adapter adds the legacy copy flag.
// Adapters borrow storage. Moving pointer words never transfers ownership of command blocks.
typedef char RingPointerIteratorSize[sizeof(RingPointerIterator) == 16 ? 1 : -1];
typedef char RingPointerIteratorWithFlagSize[sizeof(RingPointerIteratorWithFlag) == 20 ? 1 : -1];
// Measure wrapped pointer-iterator distance relative to the saved ring boundary.
static inline int RingPointerIterator_Distance(RingPointerIteratorWithFlag* a, RingPointerIteratorWithFlag* b) {
    void** end = b->end;
    int distance = b->cur - a->cur;
    if (b->cur >= end) {
        if (a->cur < end)
            distance -= b->limit - b->base;
    } else if (a->cur >= end)
        distance += b->limit - b->base;
    return distance;
}
// Copy iterator words in the original volatile load order.
static inline void RingPointerIterator_CopyVolatile(RingPointerIterator* out, const volatile RingPointerIterator* r) {
    void** c = r->cur;
    void** base = r->base;
    void** end = r->end;
    void** limit = r->limit;
    out->cur = c;
    out->base = base;
    out->end = end;
    out->limit = limit;
}
// Prepare an advanced iterator, wrapping a single capacity boundary.
static inline void RingPointerIterator_PrepareAdvance(RingPointerIteratorWithFlag* tmp, RingPointerIteratorWithFlag* a,
                                                      int n) {
    int capacity;
    void** cur;
    void** base;
    void** end;
    void** limit;
    bool flag;
    base = a->base;
    limit = a->limit;
    cur = a->cur;
    end = a->end;
    capacity = limit - base;
    tmp->cur = cur;
    int index = n + (cur - base);
    flag = a->flag;
    tmp->base = base;
    tmp->end = end;
    tmp->limit = limit;
    tmp->flag = flag;
    if (index < 0)
        index += capacity;
    else if (index >= capacity)
        index -= capacity;
    tmp->cur = ((volatile RingPointerIteratorWithFlag*)tmp)->base + index;
}
// Advance the output cursor and decrement the remaining copy count.
static inline int RingPointerIterator_AdvanceOutput(RingPointerIterator* r, int n, int* distance) {
    void** advanced = r->cur + (n - 1) + 1;
    int remaining = *distance - n;
    *distance = remaining;
    r->cur = advanced;
    if (r->cur == r->limit)
        r->cur = r->base;
    return remaining;
}
// Copy a range of ring-buffer pointers, splitting the destination copy at the wrap boundary.
void RingBuffer32_CopyWrappedRange(void* out, void* left, void* right, void* ring) {
    RingPointerIteratorWithFlag* sourceBegin = (RingPointerIteratorWithFlag*)left;
    RingPointerIteratorWithFlag* sourceEnd = (RingPointerIteratorWithFlag*)right;
    RingPointerIterator* destination = (RingPointerIterator*)ring;
    if (sourceBegin->cur != sourceEnd->cur) {
        struct Frame {
            int size;
            int distance;
            RingPointerIteratorWithFlag tmp;
            RingPointerIteratorWithFlag next;
        } f;
        int remaining = RingPointerIterator_Distance(sourceBegin, sourceEnd);
        f.distance = remaining;
        do {
            const int* amount = &f.size;
            f.size = destination->limit - destination->cur;
            if (f.size >= remaining)
                amount = &f.distance;
            int n = *amount;
            RingPointerIterator_PrepareAdvance(&f.tmp, sourceBegin, n);
            f.next.cur = f.tmp.cur;
            f.next.base = f.tmp.base;
            f.next.end = f.tmp.end;
            f.next.limit = f.tmp.limit;
            f.next.flag = f.tmp.flag;
            bool nextFlag = f.tmp.flag;
            void** dst = destination->cur;
            void** current = sourceBegin->cur;
            void** first = sourceBegin->base;
            void** copyLimit = sourceBegin->limit;
            while (current != f.next.cur) {
                *dst = *current++;
                if (current == copyLimit)
                    current = first;
                ++dst;
            }
            remaining = RingPointerIterator_AdvanceOutput(destination, n, &f.distance);
            if (remaining == 0)
                break;
            sourceBegin->cur = f.next.cur;
            sourceBegin->base = f.next.base;
            sourceBegin->end = f.next.end;
            sourceBegin->limit = f.next.limit;
            sourceBegin->flag = nextFlag;
        } while (true);
    }
    RingPointerIterator_CopyVolatile((RingPointerIterator*)out, destination);
}

extern void TutorialCommandDeque_ReserveBackBlocks(void*, int);
union TutorialCommandRecord {
    unsigned int words[10];
    TutorialCommandView command;
};
// Construct one command record in place when its destination is non-null.
static inline void TutorialCommand_ConstructCopy(TutorialCommandRecord* item, const TutorialCommandRecord& source) {
    if (item != 0) {
        try {
            *item = source;
        } catch (...) {
            throw;
        }
    }
}
// Load the ring capacity word used by iterator initialization.
static inline unsigned int RingStorage_LoadCapacity(unsigned int* cap) {
    return *cap;
}
// Build a pointer iterator from the ring storage words.
static inline void RingStorage_MakeBegin(RingPointerIterator* iter, unsigned int* fields) {
    unsigned int count = RingStorage_LoadCapacity(fields + 3);
    void** base = (void**)fields[0];
    void** limit = base + count;
    unsigned int middle = *(volatile unsigned int*)(fields + 1);
    void** cur = base + fields[1];
    void** end = base + middle;
    RingPointerIterator* out = iter;
    out->base = base;
    out->cur = cur;
    out->end = end;
    out->limit = limit;
}
struct CommandBlockIterator : RingPointerIterator {
    // Borrow the beginning of an existing block ring.
    CommandBlockIterator(unsigned int* fields) {
        RingStorage_MakeBegin(this, fields);
    }
};
// Advance a block iterator by a signed block offset with wrapping.
static inline void CommandBlockIterator_Advance(CommandBlockIterator* iter, int n) {
    int size = ((int)iter->limit - (int)iter->base) / 4;
    int index = n + ((int)iter->cur - (int)iter->base) / 4;
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    iter->cur = ((volatile RingPointerIterator*)iter)->base + index;
}
#pragma push
#pragma use_lmw_stmw on
// Append copies of an opcode with a byte action or text payload and an integer delay encoded in the ABI pointer.
void CTutorial_AppendRepeatedCommands(void* object, int mode, const void* input, unsigned int* encodedDelayMilliseconds,
                                      int count) {
    TutorialCommandRecord entry;
    entry.command.opcode = (unsigned int)mode;
    entry.command.delayMilliseconds = (unsigned int)encodedDelayMilliseconds;
    if (input == 0) {
        *(unsigned char*)&entry.command.payload.action = 0;
    } else {
        switch (mode) {
        case 1:
        case 2:
        case 4:
            *(unsigned char*)&entry.command.payload.action = *(const unsigned char*)input;
            break;
        default:
            strncpy(entry.command.payload.text, (const char*)input, kCommandTextBytes);
            break;
        }
    }
    unsigned int* fields = (unsigned int*)object;
    int i = 0;
    if (count > 0) {
        for (; i < count; ++i) {
            volatile unsigned char flag;
            unsigned int capacity = fields[kTutorialWord_commandBlockCount];
            unsigned int zero = 0;
            unsigned int max;
            unsigned int used = fields[kTutorialWord_commandReadOffset] + fields[kTutorialWord_pendingCommandCount];
            flag = zero;
            if (capacity == 0)
                max = zero;
            else
                max = (capacity - 1) * kCommandsPerBlock;
            if (max - used == 0)
                TutorialCommandDeque_ReserveBackBlocks((char*)&AsCTutorial(object)->commandReadOffset, 1);
            CommandBlockIterator iter(fields + kTutorialWord_commandBlocks);
            unsigned int total = fields[kTutorialWord_pendingCommandCount] + fields[kTutorialWord_commandReadOffset];
            CommandBlockIterator_Advance(&iter, total / kCommandsPerBlock);
            void* block = fields[kTutorialWord_commandBlockCount] == 0 ? 0 : *iter.cur;
            TutorialCommandRecord* item =
                (TutorialCommandRecord*)((char*)block + (total % kCommandsPerBlock) * sizeof(TutorialCommandView));
            TutorialCommand_ConstructCopy(item, entry);
            ++fields[kTutorialWord_pendingCommandCount];
        }
    }
}
#pragma pop

// Swap the four storage words of two ring buffers without allocating or copying their elements.
void TutorialBlockRing_SwapStorageWords(void* a, void* b) {
    unsigned int* left = (unsigned int*)a;
    unsigned int* right = (unsigned int*)b;
    unsigned int temp = left[0];
    left[0] = right[0];
    right[0] = temp;
    temp = left[1];
    left[1] = right[1];
    right[1] = temp;
    temp = left[2];
    left[2] = right[2];
    right[2] = temp;
    temp = left[3];
    left[3] = right[3];
    right[3] = temp;
}

struct RingWordIterator {
    unsigned int* cur;
    unsigned int* base;
    unsigned int* end;
    unsigned int* limit;
};
// Advance a word iterator by a signed offset with wrapping.
static inline void RingWordIterator_Advance(RingWordIterator* tmp, int index) {
    int count = tmp->limit - tmp->base;
    index += tmp->cur - tmp->base;
    if (index < 0) {
        index += count;
    } else if (index >= count) {
        index -= count;
    }
    tmp->cur = tmp->base + index;
}
// Copy a ring iterator and move it backward by the requested element offset.
void TutorialBlockRing_IteratorSubtractOffset(void* dst, const void* src, int offset) {
    int negOffset = -offset;
    RingWordIterator tmp = *(const RingWordIterator*)src;
    RingWordIterator_Advance(&tmp, negOffset);
    *(RingWordIterator*)dst = tmp;
}

extern int RingBuffer32_IteratorDistance(void*, void*);
extern void* TutorialBlockRing_CopyIteratorWithFlag(void*, const void*);
extern unsigned int* RingBuffer32_GetCapacityAddressForInsert(void*);
extern void* RingBuffer32_MakeBeginIterator(void*, void*);
extern void* RingBuffer32_MakeEndIteratorVolatile(void*, void*);
extern void* RingIterator32_Copy(void*, void*);
extern unsigned int RingIterator32_GetCurrentPointer(void*);
extern void RingIterator32_RetreatInPlace(void*, unsigned int);
extern void RingIterator32_SubtractOffset(void*, void*, unsigned int);
extern void RingIterator32_CopyBackward(void*, void*, void*, void*);
extern void RingIterator32_AddOffset(void*, void*, unsigned int);
extern void RingIterator32_CopyForward(void*, void*, void*, void*);
extern void RingIterator32_CopyWords(void*, void*);
extern void* RingBuffer32_GetCapacityAddressForAllocation(void*);
extern void RingBuffer32_InitializeHeader(void*, void*);
extern void* RingBuffer32_CalculateGrowth(void*, unsigned int);
extern void RingBuffer32_AllocateWords(void*, unsigned int, void*);
extern void* RingBuffer32_GetAllocationWord(void*);
extern void RingBuffer32_ConstructWord(void*, void*, void*);
extern void RingBuffer32_AppendIteratorRange(void*, void*, void*);
extern void RingBuffer32_CopyWrappedRangeToTail(void*, void*, void*);
extern void RingBuffer32_ClearCount(void*);
extern void Swap_Int32ForRingStorage(void*, void*);
extern void Swap_Int32(void*, void*);
extern unsigned int RingBuffer32_GetCapacityAddress(void*);
extern void RingBuffer32_CopyWrappedRange(void*, void*, void*, void*);
extern void RingBufferIterator_Increment(void*);
extern void* RingBufferIterator_Dereference(void*);
extern void RingBuffer32_Release(void*, int);

struct LocalA36C {
    unsigned int data, start, used, capacity;
    // Initialize temporary word-ring storage using the supplied allocator view.
    LocalA36C(void* allocator) {
        RingBuffer32_InitializeHeader(this, allocator);
    }
    // Release temporary ring storage after insertion or exceptional exit.
    ~LocalA36C() {
        RingBuffer32_Release(this, -1);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Insert a pointer range by shifting the nearer side, or allocate replacement storage when capacity is insufficient.
// Insert the half-open [second, third) word range before iterator first in object.
// flag is ignored. Input iterators and source storage are borrowed; copied pointer
// values do not acquire ownership of their pointees. Growth may invalidate iterators.
void RingBuffer32_InsertRange(void* object, void* first, void* second, void* third, void* flag) {
    (void)flag;
    struct Frame {
        RingPointerIterator oldBegin, oldEnd, copy, insert;
        RingPointerIterator out, next, last, back, end;
        RingPointerIterator dest, moved, part, result, temp, work;
        RingPointerIteratorWithFlag c, d, a, b;
        RingStorageWords ring;
    } frame;
    // Phase 1: measure the source range and locate the destination logical index.
    void* right = TutorialBlockRing_CopyIteratorWithFlag(&frame.a, third);
    int count = RingBuffer32_IteratorDistance(TutorialBlockRing_CopyIteratorWithFlag(&frame.b, second), right);
    if (count != 0) {
        unsigned int* obj = (unsigned int*)object;
        unsigned int start = ((RingStorageWords*)obj)->data + ((RingStorageWords*)obj)->start * 4;
        int index = (int)((unsigned int)((RingPointerIterator*)first)->cur - start) / 4;
        if ((unsigned int)((RingPointerIterator*)first)->cur < start)
            index += *RingBuffer32_GetCapacityAddressForInsert(object);
        unsigned int limit = RingBuffer32_GetUsableCapacity(object);
        // Phase 2a: if capacity permits, open a gap by shifting the shorter prefix/suffix.
        if ((unsigned int)count <= limit && ((RingStorageWords*)obj)->used <= limit - (unsigned int)count) {
            RingBuffer32_MakeBeginIterator(&frame.work, object);
            unsigned int remain = ((RingStorageWords*)obj)->used - index;
            if ((unsigned int)index < remain) {
                RingIterator32_Copy(&frame.temp, &frame.work);
                RingIterator32_RetreatInPlace(&frame.work, count);
                int offset = (int)(RingIterator32_GetCurrentPointer(&frame.work) - ((RingStorageWords*)obj)->data) / 4;
                ((RingStorageWords*)obj)->used += count;
                ((RingStorageWords*)obj)->start = offset;
                if (index != 0) {
                    void* target = RingIterator32_Copy(&frame.dest, &frame.work);
                    RingIterator32_AddOffset(&frame.moved, &frame.temp, index);
                    RingIterator32_CopyForward(&frame.result, RingIterator32_Copy(&frame.part, &frame.temp),
                                               &frame.moved, target);
                    RingIterator32_CopyWords(&frame.work, &frame.result);
                }
            } else {
                ((RingStorageWords*)obj)->used += count;
                RingBuffer32_MakeEndIteratorVolatile(&frame.end, object);
                RingIterator32_CopyWords(&frame.work, &frame.end);
                if (remain != 0) {
                    void* target = RingIterator32_Copy(&frame.out, &frame.work);
                    RingIterator32_SubtractOffset(&frame.next, &frame.work, count);
                    RingIterator32_SubtractOffset(&frame.last, &frame.work, count + remain);
                    RingIterator32_CopyBackward(&frame.back, &frame.last, &frame.next, target);
                    RingIterator32_CopyWords(&frame.work, &frame.back);
                }
                RingIterator32_RetreatInPlace(&frame.work, count);
            }
            first = RingIterator32_Copy(&frame.copy, &frame.work);
            object = TutorialBlockRing_CopyIteratorWithFlag(&frame.c, third);
            RingBuffer32_CopyWrappedRange(&frame.insert, TutorialBlockRing_CopyIteratorWithFlag(&frame.d, second),
                                          object, first);
        } else {
            // Phase 2b: construct inserted words in replacement storage, then copy suffix/prefix.
            RingBuffer32_InitializeHeader(&frame.ring, RingBuffer32_GetCapacityAddressForAllocation(object));
            EH5Cleanup eh5Cleanup5(&frame.ring, (void (*)(void*, int))RingBuffer32_Release);
            void* allocation = RingBuffer32_CalculateGrowth(object, count);
            RingBuffer32_AllocateWords(&frame.ring, ((RingStorageWords*)obj)->used + count, allocation);
            frame.ring.start = index;
            unsigned int* cursor = (unsigned int*)frame.ring.data + index;
            while (count != 0) {
                unsigned int value = ((unsigned int (*)(void*))RingBufferIterator_Dereference)(second);
                RingBuffer32_ConstructWord(RingBuffer32_GetAllocationWord(&frame.ring), cursor, &value);
                unsigned int used = frame.ring.used;
                --count;
                ++cursor;
                frame.ring.used = used + 1;
                ((void (*)(void*, unsigned int))RingBufferIterator_Increment)(second, used);
            }
            RingBuffer32_MakeEndIteratorVolatile(&frame.oldEnd, object);
            RingBuffer32_AppendIteratorRange(&frame.ring, first, &frame.oldEnd);
            RingBuffer32_MakeBeginIterator(&frame.oldBegin, object);
            RingBuffer32_CopyWrappedRangeToTail(&frame.ring, &frame.oldBegin, first);
            // Phase 3: commit all storage words; cleanup releases the previous allocation.
            RingBuffer32_ClearCount(object);
            Swap_Int32ForRingStorage(object, &frame.ring);
            Swap_Int32(obj + 1, &frame.ring.start);
            Swap_Int32(obj + 2, &frame.ring.used);
            unsigned int capacity = RingBuffer32_GetCapacityAddress(&frame.ring);
            Swap_Int32(RingBuffer32_GetCapacityAddressForInsert(object), (void*)capacity);
            {
                eh5Cleanup5.release();
                RingBuffer32_Release(&frame.ring, -1);
            }
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Copy the insertion iterators into ABI scratch records before inserting the range.
void RingBuffer32_InsertRangeWithIteratorCopies(void* a, void* b, void* c, void* d) {
    struct Frame {
        unsigned char flag;
        unsigned char pad2[7];
        unsigned int first[4];
        unsigned int third[4];
        unsigned char pad3[4];
        unsigned int second[4];
    } frame;
    unsigned int d0, zero, b0, d1, d2, d3, c0, c1, c2, c3, b1, b2, b3;
    d0 = ((unsigned int*)d)[0];
    zero = 0;
    b0 = ((unsigned int*)b)[0];
    d1 = ((unsigned int*)d)[1];
    d2 = ((unsigned int*)d)[2];
    d3 = ((unsigned int*)d)[3];
    c0 = ((unsigned int*)c)[0];
    c1 = ((unsigned int*)c)[1];
    c2 = ((unsigned int*)c)[2];
    c3 = ((unsigned int*)c)[3];
    b1 = ((unsigned int*)b)[1];
    b2 = ((unsigned int*)b)[2];
    b3 = ((unsigned int*)b)[3];
    frame.third[0] = d0;
    frame.third[1] = d1;
    frame.third[2] = d2;
    frame.third[3] = d3;
    frame.second[0] = c0;
    frame.second[1] = c1;
    frame.second[2] = c2;
    frame.second[3] = c3;
    frame.first[0] = b0;
    frame.first[1] = b1;
    frame.first[2] = b2;
    frame.first[3] = b3;
    frame.flag = zero;
    RingBuffer32_InsertRange(a, frame.first, frame.second, frame.third, &frame.flag);
}

// Return the final occupied ring slot, wrapping the index at the storage capacity.
void* RingBuffer_GetBackPtr(void* p) {
    unsigned int* w = (unsigned int*)p;
    unsigned int a = w[1];
    unsigned int b = w[2];
    unsigned int c = w[3];
    unsigned int idx = a + b - 1;
    void* base = (void*)w[0];
    unsigned int rem = idx % c;
    return (char*)base + rem * 4;
}

// Return the signed distance between two wrapped word iterators.
int RingBuffer32_IteratorDistance(void* a, void* b) {
    unsigned int end;
    unsigned int* pb;
    unsigned int start;
    unsigned int tail;
    unsigned int first;
    unsigned int* pa;
    unsigned int last;
    int result;
    pa = (unsigned int*)a;
    pb = (unsigned int*)b;
    start = pb[0];
    first = pa[0];
    tail = pb[1];
    end = pb[2];
    last = pb[3];
    result = ((int)(start - first)) / 4;
    if (start >= end) {
        if (first >= end) {
            return result;
        }
        return result - ((int)(last - tail)) / 4;
    }
    if (first < end) {
        return result;
    }
    return result + ((int)(last - tail)) / 4;
}

extern "C" void RingBuffer32_AppendWord(void*, void*);
extern "C" void* appMallocDummy__FUl(int);
extern "C" void* __dt__9CTutorialFv(void*, int);

extern const float kFloat_6_2831855;
extern float s_twoPi_80209C3C_805E2B70;
extern int s_tutorialCommandDequeMaximumCountWord;
extern signed char s_ringBuffer25MaxSizeInitialized;
extern void* g_sharedDoc;

struct RefBaseTargets {
    unsigned int count;
};
struct RefTargets : RefBaseTargets {
    virtual ~RefTargets();
};

// Release the message dialog, record first-time stage completion and begin the stage-end sequence.
int CTutorial_RecordFirstCompletionAndBeginStageEnd(void* p) {
    RefTargets* value = (*(RefTargets**)&AsCTutorial(p)->messageDialog);
    if (value != 0) {
        unsigned int* count = (unsigned int*)value;
        *count = *count - 1;
        if (*count == 0) {
            delete value;
        }
        AsCTutorial(p)->messageDialog = 0;
    }
    if (AsCTutorial(p)->modeMessageDialog == 0) {
        void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        int offset = AsCGolfDoc(manager)->tutorialStage;
        if (*((unsigned char*)p + offset + kTutorialStageFlagsOffset) == 0) {
            (*(unsigned char*)&AsCTutorial(p)->firstStageCompletion) = 1;
        }
    }
    ((CTutorial*)p)->BeginStageCompletion();
    return 1;
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Copy four iterator words and its trailing byte flag without changing the compiler-sensitive load order.
void* TutorialBlockRing_CopyIteratorWithFlag(void* dst, const void* src) {
    unsigned int a = *(const unsigned int*)((const char*)src + 0);

    unsigned int b = *(const unsigned int*)((const char*)src + 4);

    __typeof__(a) _p225 = a;
    __typeof__(_p225) _p389 = _p225;

    *(unsigned int*)((char*)dst + 0) = _p389;
    *(unsigned int*)((char*)dst + 4) = b;
    *(unsigned int*)((char*)dst + 8) = (*(const unsigned int*)((const char*)src + 8));
    *(unsigned int*)((char*)dst + 12) = (*(const unsigned int*)((const char*)src + 12));
    *(unsigned char*)((char*)dst + 16) = (*(const unsigned char*)((const char*)src + 16));
    return dst;
}
#pragma pop

// Advance the first-slot index with wrapping and decrement the occupied-slot count.
void RingBuffer_PopFront(void* p) {
    unsigned int* words = (unsigned int*)p;
    unsigned int current = words[1] + 1;
    words[1] = current;
    if (current == words[3]) {
        words[1] = 0;
    }
    words[2] = words[2] - 1;
}

// Construct a begin iterator from the ring storage base, first index and capacity.
void TutorialBlockRing_MakeBeginIterator(void* dst, void* src) {
    unsigned int* out = (unsigned int*)dst;
    unsigned int* in = (unsigned int*)src;
    void* base = (void*)in[0];
    void* current = (char*)base + in[1] * 4;
    void* end = (char*)base + in[3] * 4;
    out[1] = (unsigned int)base;
    out[0] = (unsigned int)current;
    out[2] = (unsigned int)current;
    out[3] = (unsigned int)end;
}

struct RingWordStorage {
    unsigned int* base;
    unsigned int first;
    unsigned int used;
    unsigned int cap;
};

struct RingWordIteratorCopy {
    unsigned int* cur;
    unsigned int* base;
    unsigned int* end;
    unsigned int* limit;
};

// Return the wrapped index immediately past the occupied range.
static inline unsigned int RingWordStorage_EndIndex(const RingWordStorage* r) {
    return r->cap != 0 ? (r->first + r->used) % r->cap : 0;
}

// Construct an end iterator at the wrapped first-plus-used index.
void TutorialBlockRing_MakeEndIterator(void* dst, void* src) {
    RingWordIteratorCopy* out = (RingWordIteratorCopy*)dst;
    RingWordStorage* in = (RingWordStorage*)src;
    RingWordIteratorCopy tmp;
    unsigned int index = RingWordStorage_EndIndex(in);
    tmp.cur = in->base + index;
    tmp.base = in->base;
    tmp.end = in->base + in->first;
    tmp.limit = in->base + in->cap;
    *out = tmp;
}

// Return the storage address of the first occupied ring slot.
void* RingBuffer_GetFrontPtr(void* p) {
    int idx = *(int*)((char*)p + 4);
    void* base = *(void**)p;
    return (char*)base + idx * 4;
}

// Append one word through the shared ring-buffer insertion routine.
void TutorialBlockRing_AppendWord(void* a, void* b) {
    RingBuffer32_AppendWord(a, b);
}

extern "C" void appFreeUni__FPv(void*);
extern "C" void __dl__FPv(void*);
extern "C" // PRECONDITION: object is a live CTutorial with a valid block ring and validated
    // command payloads. sscanf return counts are ignored, so malformed text may leave
    // output locals uninitialized. %s is unbounded; actor names must fit 31 bytes + NUL.
    // messageDialog holds one reference; close releases it. modeMessageDialog prevents
    // all execution until its result callback releases it. Callback descriptors are
    // three ABI words, adjusted for self's secondary interface at +0x18.
    // Zero-delay chains recurse; the queue must eventually wait or end.
    void CTutorial_ExecuteCurrentCommand(void*);
extern "C" void TutorialDialog_CreateForMessageIndex(int, int, void*, float);
extern "C" void* Runtime_AdjustMemberPointer(int, void*, void*);
extern void* g_mouseCursor;
extern const float kFloat_0_5_Tutorial;
extern const float kFloat_119_0;
extern const float kFloat_82_InGameDialog;
extern char s_tutorialModeDialogCallbackData[0x2C];

struct ReleaseD260 {
    unsigned int count;
    virtual ~ReleaseD260();
};

// Release the message dialog and resume command execution when aim alignment is complete.
int CTutorial_ResumeCommandAfterAimAlignment(void* p, int arg) {
    ReleaseD260* value = (*(ReleaseD260**)&AsCTutorial(p)->messageDialog);
    if (value != 0) {
        value->count = value->count - 1;
        if (value->count == 0) {
            delete value;
        }
        (*(unsigned int**)&AsCTutorial(p)->messageDialog) = 0;
    }
    if (arg == 2)
        return 1;
    ((CMouseCursor*)g_mouseCursor)->SavePosition();
    if (AsCTutorial(p)->aimAlignmentCount == 6) {
        AsCTutorial(p)->aimAlignmentCount = 0;
        ((CTutorial*)p)->BeginStageCompletion();
        return 1;
    }
    CTutorial_ExecuteCurrentCommand(p);
    return 1;
}

struct BlockRingLayout {
    int unk0;
    int unk4;
    void** data;
    unsigned int first;
    unsigned int used;
    unsigned int cap;
};

// Subtract the second count word from the first in place.
static inline void sub80209C3C(unsigned int* a, unsigned int* b) {
    *a = *a - *b;
}

#pragma push
#pragma use_lmw_stmw on
// Free all command blocks in a temporary queue, then free its ring storage and optionally the queue itself.
void* TutorialBlockRing_dtor(void* p, int deallocate) {
    BlockRingLayout* r = (BlockRingLayout*)p;
    if (r != 0) {
        unsigned int u = r->used;
        r->unk4 = 0;
        if (u != 0) {
            void** base;
            void** end;
            {
                unsigned int idx = r->cap != 0 ? (r->first + r->used) % r->cap : 0;
                end = r->data + idx;
            }
            base = r->data;
            void** cur = base + r->first;
            void** limit = base + r->cap;
            while (end != cur) {
                if (*cur != 0)
                    appFreeUni__FPv(*cur);
                ++cur;
                if (cur == limit)
                    cur = base;
            }
        }
        if (&r->data != 0 && &r->data != 0 && &r->data != 0 && &r->data != 0 && &r->data != 0 && &r->data != 0) {
            if (r->data != 0) {
                r->used = r->used - r->used;
                appFreeUni__FPv(r->data);
            }
        }
        if (deallocate > 0)
            __dl__FPv(p);
    }
    return p;
}
#pragma pop

// Replace the current message with the mode-specific tutorial dialog and attach its callback.
int CTutorial_ShowModeMessageDialog(void* p) {
    void** fields = (void**)p;
    void* value = fields[0x5C / 4];
    if (value != 0) {
        unsigned int* count = (unsigned int*)value;
        *count = *count - 1;
        if (*count == 0)
            ((void (*)(void*, int))(*(void***)((char*)value + 4))[2])(value, 1);
        fields[0x5C / 4] = 0;
    }
    void* manager = g_sharedDoc;
    int type = AsCSharedDoc(manager)->gameMode == 2 ? 8 : 0x47;
    TutorialDialog_CreateForMessageIndex(0, type, ((char*)&AsCTutorial(p)->modeMessageDialog), kFloat_0_5_Tutorial);
    void* owner = p;
    if (p != 0)
        owner = ((char*)&AsCTutorial(p)->secondaryVtable);
    char temp
        [0xC]; // 12-byte scratch for Runtime_AdjustMemberPointer (as at its other call sites); the result is its return value, not the first word of this buffer
    unsigned int rect[3];
    unsigned int* source = (unsigned int*)Runtime_AdjustMemberPointer(-24, &s_tutorialModeDialogCallbackData, temp);
    rect[0] = source[0];
    rect[1] = source[1];
    rect[2] = source[2];
    void* pane = fields[0x60 / 4];
    // Original 0x8020DE04-0x8020DE0C: lwz r3,0x60(r30); lwz r12,0x4(r3); lwz r12,0x4c(r12): the vtable
    // pointer is the word at pane+4 (same call pattern at 0x8020B630 in CTutorial_ExecuteCurrentCommand). Ours loaded
    // *(pane+0) and read +4 from there, an extra indirection that calls through a wrong pointer.
    ((void (*)(void*, unsigned int*, void*))(*(void***)((char*)pane + 4))[0x4C / 4])(pane, rect, owner);
    pane = fields[0x60 / 4]; // 0x8020DE18: the original reloads this+0x60 after the call
    *(float*)((char*)pane + 8) = kFloat_119_0;
    *(float*)((char*)pane + 12) = kFloat_82_InGameDialog;
    ((CMouseCursor*)g_mouseCursor)->moveToActive = 0; // original +0x108
    return 1;
}

#pragma opt_propagation off
#pragma opt_common_subs off
// Clear the occupied count, free owned ring storage and optionally delete the wrapper.
void* dtor_80209DD0(void* p, int deallocate) {
    // Original 0x80209DD0: retain each base-destructor null guard.
    if (p != 0) {
        if (p != 0) {
            if (p != 0) {
                if (p != 0) {
                    void* value = *(void**)p;
                    if (value != 0) {
                        unsigned int* count = (unsigned int*)((char*)p + 8);
                        unsigned int t = *count;
                        *count -= t;
                        appFreeUni__FPv(value);
                    }
                }
            }
        }
        if (deallocate > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Free owned ring storage through the original nested base-destructor null guards.
void* dtor_80209D4C(void* p, int deallocate) {
    // Original 0x80209D4C: retain each base-destructor null guard.
    if (p != 0) {
        if (p != 0) {
            if (p != 0) {
                if (p != 0) {
                    if (p != 0) {
                        if (p != 0) {
                            void* value = *(void**)p;
                            if (value != 0) {
                                unsigned int* count = (unsigned int*)((char*)p + 8);
                                unsigned int t = *count;
                                *count -= t;
                                appFreeUni__FPv(value);
                            }
                        }
                    }
                }
            }
        }
        if (deallocate > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

#pragma opt_common_subs reset
#pragma opt_propagation reset

// Return the embedded block-ring storage/allocator subobject at the third queue word.
void* TutorialCommandDeque_GetBlockAllocatorView(void* p) {
    return (char*)p + 8;
}

// Return the word addressed by the ring iterator current pointer.
void* RingBufferIterator_Dereference(void* p) {
    return *(void**)*(void**)p;
}

struct RingA9A4 {
    unsigned int* data;
    unsigned int index;
    unsigned int size;
    unsigned int capacity;
    // Release the temporary ring's word allocation, without destroying pointed-to blocks.
    ~RingA9A4() {
        if (data != 0) {
            size -= *(volatile unsigned int*)&size;
            appFreeUni__FPv(data);
        }
    }
};

struct OuterA9A4 {
    unsigned int head0;
    unsigned int head1;
    RingA9A4 ring;
};

struct IterA9A4 {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* unused;
    unsigned int* end;
    // Leave scratch iterator words untouched until explicitly populated.
    IterA9A4() {}
    // Store all four iterator pointers in the compiler-required order.
    IterA9A4(unsigned int* c, unsigned int* b, unsigned int* u, unsigned int* e) {
        volatile IterA9A4* out = this;
        out->current = c;
        out->begin = b;
        out->unused = u;
        out->end = e;
    }
    // Copy the full iterator, including its logical wrap origin.
    IterA9A4(const IterA9A4& x) {
        unsigned int* c = x.current;
        unsigned int* b = x.begin;
        unsigned int* u = x.unused;
        unsigned int* e = x.end;
        volatile IterA9A4* out = this;
        out->current = c;
        out->begin = b;
        out->unused = u;
        out->end = e;
    }
};

struct LengthErrorA9A4 {
    void* vtable;
    char* text;
    void* shared;
};

extern "C" void __dla__FPv(void*);
extern char s_sharedCharDeleterVtable[];

struct GuardA9A4 {
    char* value;
    unsigned char* flag;
    // Own the exception message until its shared deleter record is ready.
    GuardA9A4(char* p, unsigned char* f) : value(p), flag(f) {}
    // Release the exception message only if ownership was not transferred.
    ~GuardA9A4() {
        if (value)
            __dla__FPv(value);
    }
};

extern "C" void __dla__FPv(void*);
extern "C" void* __nwa__FUl(int);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
extern "C" void __dt__Q23std12length_errorFv(void*);
extern "C" char* strcpy(char*, const char*);
extern signed char s_uintVectorMaxSizeInitialized;
extern unsigned int s_fourByteVectorMaxElements;
extern const char tutorialCommandStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct FrameA9A4 {
    IterA9A4 source, first, last, next, mid;
    IterA9A4 lastSecond, nextSecond, fSecond;
    IterA9A4 midFirst, midCopy, fFirst;
    LengthErrorA9A4 error;
    unsigned int padding;
    IterA9A4 firstCopy, lastCopy;
    RingA9A4 local;
};

// Copy iterator fields using volatile stores to preserve the original scratch-record ordering.
static inline void setA9A4(IterA9A4& out, const IterA9A4& x) {
    unsigned int* c = x.current;
    unsigned int* b = x.begin;
    unsigned int* u = x.unused;
    unsigned int* e = x.end;
    volatile IterA9A4* p = &out;
    p->current = c;
    p->begin = b;
    p->unused = u;
    p->end = e;
}

// Measure a word-iterator range across the ring boundary.
static inline int distanceA9A4(const IterA9A4& a, const IterA9A4& b) {
    int n = b.current - a.current;
    if (b.current >= b.unused) {
        if (a.current < b.unused)
            n -= b.end - b.begin;
    } else {
        if (a.current >= b.unused)
            n += b.end - b.begin;
    }
    return n;
}

// Advance a temporary word iterator with single-wrap correction.
static inline void advanceA9A4(IterA9A4& a, int n) {
    int count = a.end - a.begin;
    n += a.current - a.begin;
    if (n < 0)
        n += count;
    else if (n >= count)
        n -= count;
    ((volatile IterA9A4*)&a)->current = a.begin + n;
}

// Copy a wrapped word-iterator range into contiguous destination storage.
static inline void copyA9A4(const IterA9A4& last, IterA9A4& f, unsigned int* dst) {
    unsigned int* stop = last.current;
    bool side = last.current >= last.unused;
    while (f.current != stop) {
        int n;
        unsigned int* current = ((volatile IterA9A4*)&f)->current;
        if ((current >= f.unused) == side)
            n = stop - current;
        else
            n = f.end - f.current;
        current = ((volatile IterA9A4*)&f)->current;
        unsigned int* end = current + n;
        unsigned int bytes = ((int)((unsigned int)end - (unsigned int)current) / 4) * 4;
        memmove(dst, current, bytes);
        dst = (unsigned int*)((char*)dst + bytes);
        f.current += n - 1;
        f.current++;
        if (f.current == f.end)
            f.current = f.begin;
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Initialize an empty command-block queue using the original temporary-ring copy scaffolding.
void* TutorialCommandDeque_InitializeEmptyBlockRing(void* a, void*) {
    // Phase 1: zero both the temporary ring and the output deque header.
    FrameA9A4 frame;
    RingA9A4& local = frame.local;
    volatile RingA9A4* init = &local;
    unsigned int zero = 0;
    unsigned int initialCapacity = 0;
    init->capacity = initialCapacity;
    init->data = (unsigned int*)initialCapacity;
    init->index = initialCapacity;
    init->size = initialCapacity;
    RingA9A4* ringp = (RingA9A4*)(((unsigned int*)a) + 2);
    RingA9A4& ring = *ringp;
    ((unsigned int*)a)[0] = initialCapacity;
    ((unsigned int*)a)[1] = initialCapacity;
    ring.data = (unsigned int*)initialCapacity;
    ring.index = initialCapacity;
    ring.size = initialCapacity;
    ring.capacity = initialCapacity;
    unsigned int index;
    if (!zero)
        index = zero;
    else
        index = (local.index + local.size) % zero;
    unsigned int startIndex = local.index;
    unsigned int lastIndex = index << 2;
    unsigned int* start = local.data;
    unsigned int capacity = ((volatile RingA9A4*)&local)->capacity;
    IterA9A4& last = frame.last;
    IterA9A4& first = frame.first;
    IterA9A4& source = frame.source;
    volatile IterA9A4* out = &last;
    out->current = (unsigned int*)((char*)start + lastIndex);
    out->begin = start;
    out->unused = start + startIndex;
    out->end = start + capacity;
    out = &first;
    out->current = last.unused;
    out->begin = last.begin;
    out->unused = last.unused;
    out->end = last.end;
    setA9A4(source, first);
    setA9A4(frame.lastCopy, last);
    setA9A4(frame.firstCopy, first);
    // Phase 2: retain the generic copy/length-check path, even for this empty source.
    int n = distanceA9A4(frame.firstCopy, frame.lastCopy);
    if (n) {
        if (!s_uintVectorMaxSizeInitialized) {
            s_fourByteVectorMaxElements = 0x3ffffffe;
            s_uintVectorMaxSizeInitialized = 1;
        }
        if ((unsigned int)n > s_fourByteVectorMaxElements) {
            LengthErrorA9A4& error = frame.error;
            unsigned char flag = 0;
            const char* message = (tutorialCommandStrings + kTutorialString_circularDequeLengthError);
            error.vtable = __vt__Q23std11logic_error;
            unsigned int len = strlen(message) + 1;
            error.text = (char*)__nwa__FUl((len << 1) - len);
            {
                char* str = error.text;
                GuardA9A4 guard(str, &flag);
                struct RefA9A4 {
                    void* vt;
                    int a, b;
                    char* str;
                };
                RefA9A4* ref = (RefA9A4*)__nw__FUl(16);
                if (ref) {
                    ref->a = 1;
                    ref->b = 1;
                    ref->vt = s_sharedCharDeleterVtable;
                    ref->str = str;
                }
                error.shared = ref;
                guard.value = 0;
            }
            strcpy(error.text, message);
            error.vtable = __vt__Q23std12length_error;
            __throw((tutorialCommandStrings + kTutorialString_lengthErrorTypeName), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }
        ring.data = (unsigned int*)appMallocDummy__FUl((n + 1) * 4);
        ring.capacity = n + 1;
        if (ring.data) {
            unsigned int* dst = ring.data + (ring.index + ring.size) % (n + 1);
            int remaining = distanceA9A4(source, last);
            int available = ring.data + ring.capacity - dst;
            const int* chosen = remaining < available ? &remaining : &available;
            available = *chosen;
            unsigned int* sourceBegin = source.begin;
            unsigned int* sourceUnused = source.unused;
            unsigned int* sourceEnd = source.end;
            ring.size += remaining;
            volatile int* amount = &available;
            IterA9A4& mid = frame.mid;
            volatile IterA9A4* m = &mid;
            m->current = source.current;
            m->begin = sourceBegin;
            m->unused = sourceUnused;
            m->end = sourceEnd;
            advanceA9A4(mid, *amount);
            setA9A4(frame.midFirst, mid);
            setA9A4(frame.midCopy, frame.midFirst);
            setA9A4(frame.fFirst, first);
            copyA9A4(frame.midCopy, frame.fFirst, dst);
            remaining -= *amount;
            if (remaining) {
                IterA9A4& next = frame.next;
                volatile IterA9A4* q = &next;
                q->current = source.current;
                q->begin = sourceBegin;
                q->unused = sourceUnused;
                q->end = sourceEnd;
                advanceA9A4(next, *amount);
                setA9A4(frame.nextSecond, next);
                setA9A4(frame.lastSecond, last);
                setA9A4(frame.fSecond, frame.nextSecond);
                copyA9A4(frame.lastSecond, frame.fSecond, ring.data);
            }
        }
    }
    return a;
}
#pragma opt_propagation reset
#pragma pop

// Return command capacity with one block reserved as the ring sentinel.
int TutorialCommandDeque_GetUsableCommandCapacity(void* p) {
    return ((TutorialCommandDeque*)p)->__capacity();
}

// Advance a word iterator and wrap from the limit to the storage base.
void RingBufferIterator_Increment(void* p) {
    ++*(TutorialBlockRing::iterator*)p;
}

// Lazily initialize and return the command deque maximum-count word.
int TutorialCommandDeque_GetMaximumCountWord() {
    if (s_ringBuffer25MaxSizeInitialized == 0) {
        s_tutorialCommandDequeMaximumCountWord = kCommandDequeMaxCountWord;
        s_ringBuffer25MaxSizeInitialized = 1;
    }
    return s_tutorialCommandDequeMaximumCountWord;
}

// Return the queue read-offset word through the original pointer-valued ABI.
void* TutorialCommandDeque_GetReadOffsetWord(void* p) {
    return (void*)((TutorialCommandDeque*)p)->__begin_index();
}

// Allocate storage for the requested number of 40-byte tutorial command records.
void* TutorialCommandBlock_AllocateRecords(int allocatorToken, int commandCount) {
    return app::app_allocator<TutorialCommandView>().allocate(commandCount);
}

// Initialize the tutorial module two-pi value.
void Tutorial_InitializeTwoPi() {
    s_twoPi_80209C3C_805E2B70 = kFloat_6_2831855;
}

// Adjust the secondary-interface pointer back to CTutorial before destruction.
// The secondary interface sits 0x18 bytes into the object, and a destructor
// cannot be called with a delete flag through C++ syntax, so the mangled name
// is used.
void* CTutorial::DestroyThunkMinus24(int deleting) {
    return __dt__9CTutorialFv((char*)this - 0x18, deleting);
}

// Return the embedded block-ring storage header.
void* TutorialCommandDeque_GetBlockRing(void* p) {
    return &((TutorialCommandDeque*)p)->__map();
}

extern void* IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern void* Pointer_ClearInputSuppression(void*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void);
extern void* g_inputPointer;
extern signed char s_inputManagerInitialized;
extern void* g_mouseCursor;
extern const char tutorialCommandStrings[];
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_tutorial[];

// Handle the lesson prompt result, restoring input bindings or resuming the current command.
int CTutorial_HandleLessonPromptResult(void* p, int mode) {
    void** fields = (void**)p;
    RefTargets* value = (RefTargets*)fields[0x5C / 4];
    if (0 != value) {
        unsigned int* count = (unsigned int*)value;
        *count = *count - 1;
        if (*count == 0)
            delete value;
        fields[0x5C / 4] = 0;
    }
    if (2 == mode) {
        fields[0x3C / 4] = (void*)4;
        IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_golfRuleActor), 0x23, 1, 0, 0, 0);
        Pointer_ClearInputSuppression(g_inputPointer);
        void* object = (void*)((int&)GetInputManagerSlot());
        if (object == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        unsigned int* words = (unsigned int*)object;
        for (int i = 0x1C / 4; i <= 0xB4 / 4; ++i)
            words[i] = 0;
        if (!object) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        words = (unsigned int*)object;
        ((GameInputManager*)words)->selectedPadLookupDisabled = 0;
        words = (unsigned int*)object;
        if (object == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        words = (unsigned int*)object;
        ((GameInputManager*)words)->actionBlocked[kTutorialScriptAction6] = 1;
        if (!object) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        words = (unsigned int*)object;
        ((GameInputManager*)words)->actionBlocked[kTutorialBlockedAction20] = 1;
    } else {
        void* object = (void*)((int&)GetInputManagerSlot());
        if (object == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        ((unsigned int*)object)[0x170 / 4] = 1;
        CTutorial_ExecuteCurrentCommand(p);
    }
    return 1;
}

// Release the active message and apply the input-blocking state requested by its result.
int CTutorial_HandleMessageDialogResult(void* p, int mode) {
    void* value = AsCTutorial(p)->messageDialog;
    if (value) {
        unsigned int* count = (unsigned int*)value;
        *count = *count - 1;
        if (*count == 0 && 0 != value)
            ((void (*)(void*, int))(*(void***)((char*)value + 4))[2])(value, 1);
        AsCTutorial(p)->messageDialog = 0;
    }
    IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_golfRuleActor), 0x24, 0, 0, 0, 0);
    if (2 == mode) {
        Pointer_ClearInputSuppression(g_inputPointer);
        void* object = (void*)((int&)GetInputManagerSlot());
        if (0 == object) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
            object = s_gameInputManager;
        }
        unsigned int* words = (unsigned int*)object;
        for (int i = 0x1C / 4; i <= 0xB4 / 4; ++i)
            words[i] = 0;
        object = (void*)((int&)GetInputManagerSlot());
        if (0 == object) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            object = s_gameInputManager;
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        words = (unsigned int*)object;
        ((GameInputManager*)words)->selectedPadLookupDisabled = 0;
        if (object == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            object = s_gameInputManager;
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        object = (void*)((int&)GetInputManagerSlot());
        words = (unsigned int*)object;
        ((GameInputManager*)words)->actionBlocked[kTutorialScriptAction6] = 1;
        if (0 == object) {
            if (0 == s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
            object = s_gameInputManager;
        }
        words = (unsigned int*)object;
        ((GameInputManager*)words)->actionBlocked[kTutorialBlockedAction20] = 1;
        IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor), kScreenResetShotPanels, 1,
                               0, 0, 0);
    } else {
        IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor), kScreenResetShotPanels, 0,
                               0, 0, 0);
        ((CMouseCursor*)g_mouseCursor)->SavePosition();
        CTutorial_ExecuteCurrentCommand(p);
    }
    return 1;
}

extern void* g_sharedDoc;

struct RefBaseD31C {
    unsigned int count;
};
struct RefD31C : RefBaseD31C {
    virtual ~RefD31C();
};

// Release the mode dialog, advance or refresh the lesson and reset its command queue.
int CTutorial_AdvanceStageOrRefresh(void* p, int mode) {
    RefD31C* old = (*(RefD31C**)&AsCTutorial(p)->modeMessageDialog);
    if (old != 0) {
        old->count = old->count - 1;
        if (old->count == 0) {
            delete old;
        }
        AsCTutorial(p)->modeMessageDialog = 0;
    }
    ((CMouseCursor*)g_mouseCursor)->SavePosition();
    // Original 0x8020D31C: stage five has no page transition.
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage != 5) {
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 4 && mode == 1) {
            CTutorial_ExecuteCurrentCommand(p);
            return 1;
        }
        if ((*(unsigned char*)&AsCTutorial(p)->firstStageCompletion) != 0 && mode == 1)
            ++AsCTutorial(p)->completedStageCount;
        (*(unsigned char*)&AsCTutorial(p)->firstStageCompletion) = 0;
        int value = mode == 1;
        IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                               kScreenSetTutorialCompletion, value + 1, 0, 0, 0);
        if (mode != 1) {
            int state = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
            switch (state) {
            case 1:
                IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                       kScreenHideTutorialMeter, 0, 0, 0, 0);
                break;
            case 2:
                IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                       kScreenHideTutorialGauge, 0, 0, 0, 0);
                IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                       kScreenHideTutorialAim, 0, 0, 0, 0);
                if (AsCSharedDoc(g_sharedDoc)->gameMode == 4)
                    IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                           kScreenSetCameraControlsEnabled, 0, 0, 0, 0);
                break;
            default:
                break;
            }
        }
        // Stage 4 begins fading the existing light column in; no new effect is allocated.
        if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == 4)
            IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_effectActor),
                                   kEffectLightColumnFadeIn, 0, 0, 0, 0);
        int index;
        if (AsCSharedDoc(g_sharedDoc)->gameMode == 3 &&
            AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == 1 && mode == 1) {
            // Mode 3 skips stage 2; expose the aim and gauge panels immediately.
            index = 3;
            IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                   kScreenShowTutorialAimImmediately, 0, 0, 0, 0);
            IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor),
                                   kScreenShowTutorialGaugeImmediately, 0, 0, 0, 0);
        } else {
            bool step;
            void* page;
            step = mode == 1;
            page = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            index = step + AsCGolfDoc(page)->tutorialStage;
        }
        // Rebuild the chosen lesson, then reset panels with p0=0: this also
        // returns the HUD to aim phase (p0=1 would preserve the current phase).
        ((CTutorial*)p)->ResetLessonCommands(index);
        IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_screenActor), kScreenResetShotPanels, 0,
                               0, 0, 0);
        // On stage 4/5 completion, select BGM from game/ground state and notify Voice.
        if (mode == 1) {
            int state = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
            if (state == 4 || state == 5)
                IActor_DispatchMessage(p, p, (tutorialCommandStrings + kTutorialString_golfRuleActor),
                                       kGolfRuleRefreshBgmAndVoice, 0, 0, 0, 0);
        }
    }
    return 1;
}

// Enqueue copied payloads using a duration in MILLISECONDS, not a pointer.
// Zero delay executes the successor immediately; -1 resumes on a callback/event.
// Action opcodes copy one byte, text opcodes copy up to 32 bytes. repeatCount
// duplicates the complete record; this call retains neither payload nor text.
static inline void AppendTutorialCommand(void* tutorial, int opcode, const void* payload, int delayMilliseconds,
                                         int repeatCount = 1) {
    CTutorial_AppendRepeatedCommands(tutorial, opcode, payload, (unsigned int*)delayMilliseconds, repeatCount);
}

extern char* String_FormatTemporary(const char*, ...);
extern void* CourseData_FindNamedPoint(void*, const char*);
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Send a no-value Screen message after its queued delay.
static inline void AppendLessonScreenMessage(void* tutorial, char*& formattedText, int message, int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_screenMessageFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Send one integer to Screen; message enums identify the affected control.
static inline void AppendLessonScreenValue(void* tutorial, char*& formattedText, int message, int value,
                                           int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_screenValueFormat, message, value);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Send a zero value to a Screen control using the existing fixed-zero format.
static inline void AppendLessonScreenDisable(void* tutorial, char*& formattedText, int message, int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_screenDisableFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a Camera input-policy value; the delay applies after dispatch.
static inline void AppendLessonCameraValue(void* tutorial, char*& formattedText, int message, int value,
                                           int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_cameraValueFormat, message, value);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a no-value GolfRule request; controller-slot enums retain unresolved operations.
static inline void AppendLessonGolfRuleMessage(void* tutorial, char*& formattedText, int message,
                                               int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleMessageFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a zero-valued GolfRule control request.
static inline void AppendLessonGolfRuleDisable(void* tutorial, char*& formattedText, int message,
                                               int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleDisableFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a one-valued GolfRule control request.
static inline void AppendLessonGolfRuleEnable(void* tutorial, char*& formattedText, int message,
                                              int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleEnableFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue two integer GolfRule operands without changing their order or units.
static inline void AppendLessonGolfRuleValues(void* tutorial, char*& formattedText, int message, int value1, int value2,
                                              int delayMilliseconds) {
    formattedText =
        String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleValuesFormat, message, value1, value2);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Enable a GolfRule control and pass its two additional integer operands.
static inline void AppendLessonGolfRuleEnabledValues(void* tutorial, char*& formattedText, int message, int value1,
                                                     int value2, int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleEnabledValuesFormat,
                                           message, value1, value2);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Enable a GolfRule control with one additional integer operand.
static inline void AppendLessonGolfRuleEnabledValue(void* tutorial, char*& formattedText, int message, int value,
                                                    int delayMilliseconds) {
    formattedText =
        String_FormatTemporary(tutorialCommandStrings + kTutorialString_golfRuleEnabledValueFormat, message, value);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a one-valued Grid request; Grid message semantics remain external.
static inline void AppendLessonGridEnable(void* tutorial, char*& formattedText, int message, int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_gridEnableFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

// Queue a zero-valued Grid request; Grid message semantics remain external.
static inline void AppendLessonGridDisable(void* tutorial, char*& formattedText, int message, int delayMilliseconds) {
    formattedText = String_FormatTemporary(tutorialCommandStrings + kTutorialString_gridDisableFormat, message);
    AppendTutorialCommand(tutorial, kCommandDispatchActorMessage, formattedText, delayMilliseconds);
}

#pragma pop

#pragma push
#pragma inline_depth(16)
// Build the landing-point-four lesson sequence, including cursor and caddie messages.
void CTutorial_BuildLandingPoint4Commands(void* object) {
    // Enqueue only; payloads are copied before temporary formatted text is reused.
    // Outline: move to landing point 4 -> cursor/action/camera demonstration ->
    // dialogs 0x29/0x2a -> move/name caddie at that point -> dialogs 0x2b/0x2c.
    // Message numbers select external text; its wording is not available here.
    // Phase 1: demonstrate cursor/action/camera input at the named landing point.
    unsigned char values[4];
    values[3] = kTutorialScriptAction6;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[3], 0);
    AppendTutorialCommand(object, kCommandTargetNamedPoint, (tutorialCommandStrings + kTutorialString_landingPoint4),
                          0);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 10);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x190), 500);
    values[2] = kTutorialScriptAction8;
    AppendTutorialCommand(object, kCommandPulseAction, &values[2], 1000);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120), 500);
    char* text;
    AppendLessonScreenMessage(object, text, kScreenResetMapCameraAction, 500);
    AppendLessonScreenMessage(object, text, kScreenResetMapCameraAction, 500);
    AppendLessonCameraValue(object, text, kCameraSetZoomInputBlocked, 1, 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 1, 0);
    values[1] = 0;
    AppendTutorialCommand(object, kCommandForceAction, &values[1], 1000);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 0, 0);
    AppendLessonCameraValue(object, text, kCameraSetZoomInputBlocked, 0, 0);
    // Phase 2: explain the demonstration with blocking dialogs, then position the caddie.
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x29);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogBunkerRectFormat), 0x2A);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* data = AsCGolfDoc(manager)->courseData;
    char* pointPositionAddress = (char*)((TutorialNamedPoint*)CourseData_FindNamedPoint(
                                             data, (tutorialCommandStrings + kTutorialString_landingPoint4)))
                                     ->positionWords;
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_caddieNamedPointFormat), 4,
                                  pointPositionAddress);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_caddieMessageFormat), 2,
                                  (tutorialCommandStrings + kTutorialString_stage4CaddieName));
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    values[0] = kTutorialScriptAction6;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[0], 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog384x160Format), 0x2B, 2);
    AppendTutorialCommand(object, kCommandSizedMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 0x2C);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
}

// Build the default stage-four lesson sequence, including grid and aim demonstrations.
void CTutorial_BuildStage4DefaultCommands(void* object) {
    // Outline: dialogs 0x2e/0x2f -> scripted action 26 with screen option 0x51 ->
    // enable Grid 5 / GolfRule 0x0c -> event wait / action 0x0c -> dialog 0x30 ->
    // camera-option-11/action-3 demonstration -> dialog 0x31 -> clear grid/rule -> 0x32.
    // Phase 1: explanatory dialogs and actor setup precede scripted cursor/action input.
    unsigned char values[4];
    values[3] = kTutorialScriptAction6;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[3], 0);
    char* text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x2E);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x2F);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 1, 0);
    AppendTutorialCommand(object, kCommandWait, 0, 10);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 1);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120), 1000);
    values[2] = kTutorialScriptAction26;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[2], 900);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 0, 0);
    AppendTutorialCommand(object, kCommandFinishCursorMove, 0, 0);
    AppendLessonGridEnable(object, text, 5, 0);
    AppendLessonGolfRuleEnabledValue(object, text, kGolfRuleSetAutoShot, 0x167, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    values[1] = 0x0C;
    AppendTutorialCommand(object, kCommandPulseAction, &values[1], kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x30);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 1, 0);
    values[0] = 3;
    AppendTutorialCommand(object, kCommandForceAction, &values[0], 400);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 0, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog384x184Format), 0x31, 3);
    AppendTutorialCommand(object, kCommandSizedMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonGridDisable(object, text, 5, 0);
    AppendLessonGolfRuleDisable(object, text, kGolfRuleSetAutoShot, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 0x32);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
}

// Build the mode-three stage-four lesson sequence with its alternate golf-rule messages.
void CTutorial_BuildStage4Mode3Commands(void* object) {
    // Same dialog/cursor opening as default stage 4. Middle section instead
    // dispatches GolfRule 0x1e, 0x1f(60,60), waits, then 0x20/0x22 and waits.
    // Both branches finish with the camera-11/action-3 demo and dialogs 0x31/0x32.
    unsigned char values[3];
    values[2] = 6;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[2], 0);
    char* text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x2E);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x2F);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 1, 0);
    AppendTutorialCommand(object, kCommandWait, 0, 10);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 1);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120), 1000);
    values[1] = kTutorialScriptAction26;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[1], 900);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 0, 0);
    AppendTutorialCommand(object, kCommandFinishCursorMove, 0, 0);
    AppendLessonGridEnable(object, text, 5, 0);
    AppendLessonGolfRuleMessage(object, text, kGolfRuleRunSwingController3C, 0);
    AppendLessonGolfRuleValues(object, text, kGolfRuleConfigureAutoSwing, 0x3C, 0x3C, 0);
    AppendTutorialCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    AppendLessonGolfRuleMessage(object, text, kGolfRuleEnableSwingController4C, 0);
    AppendLessonGolfRuleMessage(object, text, kGolfRuleRunSwingController54, 0);
    AppendTutorialCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    AppendLessonGolfRuleDisable(object, text, kGolfRuleSetAutoShot, 1000);
    AppendLessonGridDisable(object, text, 5, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x30);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 1, 0);
    values[0] = 3;
    AppendTutorialCommand(object, kCommandForceAction, &values[0], 400);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 0, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog384x184Format), 0x31, 3);
    AppendTutorialCommand(object, kCommandSizedMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 0x32);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
}

// Build the introductory lesson sequence for cursor actions and swing controls.
void CTutorial_BuildStage1Commands(void* object) {
    // 1. Reset actor options and show dialogs 0/1 before scripted input begins.
    char* text;
    AppendLessonScreenValue(object, text, kScreenSetTutorialSpinControlEnabled, 0, 0);
    AppendLessonScreenValue(object, text, kScreenLegacyMessage53, 0, 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 0, 0);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 0, 0);
    AppendLessonScreenValue(object, text, kScreenSetTutorialClubChangeEnabled, 0, 0);
    AppendLessonScreenDisable(object, text, kScreenResetShotPanels, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonScreenMessage(object, text, kScreenSetTutorialCompletion, 0);
    AppendLessonScreenMessage(object, text, kScreenShowTutorialMeter, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogSwingRectFormat), 1);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    // 2. Pulse actions 1 then 0 thirteen times each, at 150-ms spacing.
    // These action indices have no verified physical-button names here.
    unsigned char values[8];
    values[7] = 1;
    AppendTutorialCommand(object, kCommandPulseAction, &values[7], 150, 0x0D);
    AppendTutorialCommand(object, kCommandWait, 0, 500);
    values[6] = 0;
    AppendTutorialCommand(object, kCommandPulseAction, &values[6], 150, 0x0D);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 2);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 1, 0);
    // 3. With camera option 11 enabled, hold actions 2/3/2, then restore it.
    values[5] = 2;
    AppendTutorialCommand(object, kCommandForceAction, &values[5], 400);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    values[4] = 3;
    AppendTutorialCommand(object, kCommandForceAction, &values[4], 600);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    values[3] = 2;
    AppendTutorialCommand(object, kCommandForceAction, &values[3], 400);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    AppendLessonCameraValue(object, text, kCameraSetDirectionalInputEnabled, 0, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 3);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    values[2] = kTutorialScriptAction8;
    AppendTutorialCommand(object, kCommandPulseAction, &values[2], 300);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 4);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialCommand(object, kCommandWait, 0, 500);
    // 4. Dialogs 3/4 lead into action 8 and two 2000-ms forced holds (9/10).
    values[1] = 9;
    AppendTutorialCommand(object, kCommandForceAction, &values[1], 2000);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    values[0] = 10;
    AppendTutorialCommand(object, kCommandForceAction, &values[0], 2000);
    AppendTutorialCommand(object, kCommandClearForcedAction, 0, 400);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 5);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    // 5. Screen option 0x50 brackets the cursor path and messages 0x45/0x46;
    // final text IDs 6/7 introduce the stage-end callback.
    AppendLessonScreenValue(object, text, kScreenSetTutorialClubChangeEnabled, 1, 0);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 10);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120), 1000);
    AppendTutorialCommand(object, kCommandWait, 0, 500);
    AppendLessonScreenMessage(object, text, kScreenResetMapCameraAction, 3000);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x190), 1000);
    AppendTutorialCommand(object, kCommandWait, 0, 500);
    AppendLessonScreenMessage(object, text, kScreenConfirmMapCameraAction, 300);
    AppendLessonScreenValue(object, text, kScreenSetTutorialClubChangeEnabled, 0, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 6);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 7);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
}

// Build the stage-two lesson sequence for mode two.
void CTutorial_BuildStage2Mode2Commands(void* object) {
    // 1. Target landing point 3, show text 9/10, and demonstrate cursor/action 26.
    unsigned char values[4];
    values[3] = kTutorialScriptAction6;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[3], 0);
    AppendTutorialCommand(object, kCommandTargetNamedPoint, (tutorialCommandStrings + kTutorialString_landingPoint3),
                          0);
    char* text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 9);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonScreenMessage(object, text, kScreenSetTutorialCompletion, 0);
    AppendLessonScreenMessage(object, text, kScreenShowTutorialAim, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x0A);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 1, 0);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 1);
    AppendTutorialCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120), 1000);
    AppendTutorialCommand(object, kCommandWait, 0, 500);
    values[2] = kTutorialScriptAction26;
    AppendTutorialCommand(object, kCommandPulseActionAlternate, &values[2], 900);
    AppendLessonScreenValue(object, text, kScreenSetTutorialAimControlEnabled, 0, 0);
    AppendLessonScreenMessage(object, text, kScreenShowTutorialGauge, 0);
    // 2. Dialogs 0x0b/0x0c frame the shot UI; GolfRule 0x0c(499,149) arms an
    // event wait. Rule 0x0d then brackets each period of player-controlled input.
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogShotRectFormat), 0x0B);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogLeftShotRectFormat), 0x0C);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonGolfRuleEnabledValues(object, text, kGolfRuleSetAutoShot, 0x1F3, 0x95, 0);
    AppendTutorialCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    AppendLessonGolfRuleEnable(object, text, kGolfRuleSetProcessingPaused, 0);
    AppendTutorialCommand(object, kCommandAllowInput, 0, 0);
    // 3. Repeat right/left UI explanations (0x0d/0x0e), parse the ignored zero
    // command and pulse action 0x0c after each. The -1 delays wait for events.
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogRightShotRectFormat), 0x0D);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonGolfRuleDisable(object, text, kGolfRuleSetProcessingPaused, 0);
    AppendTutorialCommand(object, kCommandParseInteger, (tutorialCommandStrings + kTutorialString_disabledArgument), 0);
    values[1] = 0x0C;
    AppendTutorialCommand(object, kCommandPulseAction, &values[1], kTutorialWaitForEvent);
    AppendLessonGolfRuleEnable(object, text, kGolfRuleSetProcessingPaused, 0);
    AppendTutorialCommand(object, kCommandAllowInput, 0, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogLeftShotRectFormat), 0x0E);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendLessonGolfRuleDisable(object, text, kGolfRuleSetProcessingPaused, 0);
    AppendTutorialCommand(object, kCommandParseInteger, (tutorialCommandStrings + kTutorialString_disabledArgument), 0);
    values[0] = 0x0C;
    AppendTutorialCommand(object, kCommandPulseAction, &values[0], kTutorialWaitForEvent);
    AppendLessonGolfRuleDisable(object, text, kGolfRuleSetAutoShot, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x0F);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 0x10);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
}

#pragma pop

// Append one copied command; delay is encoded as integer bits in the legacy pointer argument.
static inline void AppendTutorialRawCommand(void* object, int opcode, const void* payload, int delayMilliseconds) {
    AppendTutorialCommand(object, opcode, payload, delayMilliseconds);
}
// Format a zero-valued actor message and append it before temporary formatting storage is reused.
static inline void AppendTutorialZeroActorValue(void* object, char*& text, int formatOffset, int message,
                                                int delayMilliseconds) {
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenActor) + formatOffset, message, 0);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, delayMilliseconds);
}

// Build the repeated swing-practice lesson sequence and its input demonstrations.
void CTutorial_BuildSwingingPracticeCommands(void* object) {
    // Lesson map: reset/target/dialog 0x3e -> scripted cursor -> UI explanations
    // 0x15/0x16 -> screen 0x54 values 0/1/2 with event waits -> aim alignment ->
    // screen 0x54=3 -> second cursor demo -> GolfRule practice steps -> prompt ->
    // dialogs 0x24/0x25/0x26 -> report completed-stage count. Text meanings and
    // individual actor command semantics remain external to this queue builder.
    // Phase 1: reset screen/camera actor values and establish the practice target.
    char* text;
    unsigned char values[8];
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x52, 0);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x53, 0);
    AppendTutorialZeroActorValue(object, text, kTutorialString_cameraValueFormat, 0x0B, 0);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x51, 0);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x50, 0);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x55, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenMessageFormat), 0x48);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleValueFormat), 6, 1);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    values[2] = 6;
    AppendTutorialRawCommand(object, kCommandPulseActionAlternate, &values[2], 0);
    AppendTutorialRawCommand(object, kCommandTargetNamedPoint, (tutorialCommandStrings + kTutorialString_landingPoint3),
                             0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x3E);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenMessageFormat), 0x1E);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenMessageFormat), 0x4B);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x14);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x51, 1);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, 10);
    AppendTutorialRawCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 1);
    AppendTutorialRawCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120),
                             1000);
    AppendTutorialRawCommand(object, kCommandWait, 0, 500);
    values[1] = kTutorialScriptAction26;
    AppendTutorialRawCommand(object, kCommandPulseActionAlternate, &values[1], 900);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x51, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenMessageFormat), 0x49);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    // Phase 2: dialog steps pause the queue between each shot/aim demonstration.
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialogShotRectFormat), 0x15);
    AppendTutorialCommand(object, kCommandMessageLayout2, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog14x35RectFormat), 0x16);
    AppendTutorialCommand(object, kCommandMessageLayout4, text, kTutorialWaitForEvent);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x54, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x54, 1);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x54, 2);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x45);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x46);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialRawCommand(object, kCommandAimAlignment, (tutorialCommandStrings + kTutorialString_enabledArgument),
                             kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandAimAlignment, (tutorialCommandStrings + kTutorialString_disabledArgument),
                             0);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x48);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x54, 3);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x51, 1);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer350x250), 1);
    AppendTutorialRawCommand(object, kCommandMoveCursor, (tutorialCommandStrings + kTutorialString_pointer70x120),
                             1000);
    values[0] = kTutorialScriptAction26;
    AppendTutorialRawCommand(object, kCommandPulseActionAlternate, &values[0], 1000);
    AppendTutorialZeroActorValue(object, text, kTutorialString_screenValueFormat, 0x51, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleMessageFormat), 0x1E);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x1B);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleValuesFormat), 0x1F, 0x5F, 0x3C);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x1C);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleMessageFormat), 0x21);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 500);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x1D);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleMessageFormat), 0x22);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleDisableFormat), 0x0C);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_screenValueFormat), 0x55, 1);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, 0);
    AppendTutorialRawCommand(object, kCommandLessonPrompt, (tutorialCommandStrings + kTutorialString_enabledArgument),
                             kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandLessonPrompt, (tutorialCommandStrings + kTutorialString_disabledArgument),
                             0);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x24);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_integerFormat), 0x25);
    AppendTutorialCommand(object, kCommandMessageDialog, text, kTutorialWaitForEvent);
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_dialog400x50Format), 0x26);
    AppendTutorialCommand(object, kCommandStageEndDialog, text, kTutorialWaitForEvent);
    AppendTutorialRawCommand(object, kCommandBlockInput, (tutorialCommandStrings + kTutorialString_enabledArgument), 0);
    AppendTutorialRawCommand(object, kCommandWait, 0, 500);
    // Phase 3: report the completed-stage count after the final stage-end dialog.
    unsigned int arg = ((unsigned int*)object)[0x68 / 4];
    text = String_FormatTemporary((tutorialCommandStrings + kTutorialString_golfRuleValueFormat), 0x12, arg);
    AppendTutorialCommand(object, kCommandDispatchActorMessage, text, kTutorialWaitForEvent);
}

extern const float kTutorialDialogRightX;
extern const float kFloat_341_TutorialDialog;
extern const float kFloat_390_TutorialDialog;
extern const float kFloat_0_Tutorial;
extern const float kFloat_0_001_Tutorial;
extern const float kFloat_140_TutorialDialog;
extern const float kFloat_139_InGameDialog;
extern const float kFloat_22_TextWidget;
extern const float kFloat_128_TextWidget;
extern char tutorialMessageIndices[];
extern const unsigned int zeroInitializationWords[3];
extern char* g_view;
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern int sscanf(const char*, const char*, ...);
extern void Pointer_SuppressInput(void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void SaveData_Ctor(void*);
extern void* SaveData_GetCurrentRecord(int);
extern void SaveRecord_CopyCtor(void*, void*);
extern void SaveData_SetCurrentRecord(int, void*);

struct TutorialCallbackWords {
    unsigned int thisAdjustment, vtableOffset, function;
};
struct TutorialPositionWords {
    unsigned int xBits, yBits, zBits;
};
struct TutorialDialogReferenceCount {
    int references;
};
class TutorialDialogInterface : public TutorialDialogReferenceCount {
public:
    virtual void Destroy(int);
    virtual void m0c();
    virtual void SetDialogState(int);
    virtual void m14();
    virtual void m18();
    virtual void m1c();
    virtual void m20();
    virtual void m24();
    virtual void m28();
    virtual void m2c();
    virtual void m30();
    virtual void m34();
    virtual void m38();
    virtual void m3c();
    virtual void m40();
    virtual void m44();
    virtual void m48();
    virtual void SetResultCallback(void*, void*);
    virtual void ResetAndApplyContent(int, int, float, float);
    virtual void m54();
    virtual void m58();
    virtual void m5c();
    virtual void SetCursorGroupPosition(int, float, float);
    virtual void SetCursorGroupEnabled(int, int);
    virtual void SetCursorGroupPlayerFlag(int, int);
};
// Set the tutorial dialog rectangle origin.
static inline void TutorialDialog_SetPosition(void* o, float x, float y) {
    ((CInGameTutorialDialog*)o)->positionX = x;
    ((CInGameTutorialDialog*)o)->positionY = y;
}

// Initialize the game input singleton on demand and return its word view.
static inline int* GetTutorialInputManagerWords(int* previous = (int*)((int&)GetInputManagerSlot())) {
    if (previous == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_tutorial);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
    return (int*)((int&)GetInputManagerSlot());
}

// Resolve the active command from its block index and intra-block read offset.
static inline char* CurrentTutorialCommandBytes(char* p) {
    // 0x8020B214: tutorial ring state; retain the original signedness of each load.
    unsigned int base = (*(unsigned int*)&AsCTutorial(p)->firstCommandBlockIndex);
    unsigned int frame = (*(unsigned int*)&AsCTutorial(p)->commandReadOffset);
    unsigned int count = (*(unsigned int*)&AsCTutorial(p)->commandBlockCapacity);
    char** rows = (*(char***)&AsCTutorial(p)->commandBlocks);
    unsigned int group = frame / kCommandsPerBlock;
    unsigned int row = (base + group) % count;
    return (char*)rows[row] + (frame - group * kCommandsPerBlock) * 0x28;
}

// Borrow the active record until the executor consumes it; never retain its payload.
static inline TutorialCommandView* CurrentTutorialCommand(char* tutorial) {
    return (TutorialCommandView*)CurrentTutorialCommandBytes(tutorial);
}

// Borrow the retained dialog through its virtual interface; ownership stays on tutorial.
static inline TutorialDialogInterface* CurrentTutorialMessageDialog(char* tutorial) {
    return *(TutorialDialogInterface**)(tutorial + kTutorialMessageDialogOffset);
}

// Parse a layout payload into caller-owned scratch; malformed text is unchecked.
// x/y are offsets from (119,82), width/height are content extents in UI pixels.
static inline void ParseTutorialDialogLayout(char* tutorial, int* messageIndex, float* x, float* y, float* width,
                                             float* height) {
    sscanf(CurrentTutorialCommand(tutorial)->payload.text,
           tutorialCommandStrings + kTutorialString_dialogIndexRectFormat, messageIndex, x, y, width, height);
}

// Set all 39 primary action-blocking words while retaining the caller's singleton access order.
static inline int* SetTutorialActionBlocking(int value, int* previous = (int*)((int&)GetInputManagerSlot())) {
    int* w = GetTutorialInputManagerWords(previous);
    for (int i = 0x1C / 4; i <= 0xB4 / 4; ++i)
        w[i] = value;
    return w;
}

#pragma push
#pragma always_inline on
#pragma inline_depth(32)
// Copy the three callback ABI words with the original two-read/two-write schedule.
static inline void CopyTutorialTriple(TutorialCallbackWords& out, const void* source) {
    const volatile unsigned int* in = (const volatile unsigned int*)source;
    unsigned int first;
    unsigned int second;
    first = in[0];
    second = in[1];
    out.thisAdjustment = first;
    out.vtableOffset = second;
    out.function = in[2];
}

// Copy the named point's three coordinate bit patterns without integer/float conversion.
static inline void CopyTutorialPositionWords(TutorialPositionWords& out, const void* source) {
    const volatile unsigned int* in = (const volatile unsigned int*)source;
    unsigned int first;
    unsigned int second;
    first = in[0];
    second = in[1];
    out.xBits = first;
    out.yBits = second;
    out.zBits = in[2];
}

// Retain the forced action index and pulse its pending-input word.
static inline void ExecuteTutorialForceAction(char* tutorial) {
    int* previous = (int*)((int&)GetInputManagerSlot());
    (*(int*)&AsCTutorial(tutorial)->forcedActionIndex) = CurrentTutorialCommand(tutorial)->payload.action;
    (*(int*)((char*)GetTutorialInputManagerWords(previous) +
             (kTutorialPendingActionsOffset + AsCTutorial(tutorial)->forcedActionIndex * 4))) = 1;
}

// Parse UI coordinates and start a cursor move lasting the command delay in milliseconds.
static inline void ExecuteTutorialMoveCursor(char* tutorial) {
    float cursorX;
    float cursorY;
    // Parse UI x/y; command delay is also duration, converted ms -> seconds.
    sscanf(CurrentTutorialCommand(tutorial)->payload.text,
           (tutorialCommandStrings + kTutorialString_cursorPositionFormat), &cursorX, &cursorY);
    ((CMouseCursor*)g_mouseCursor)
        ->MoveTo(cursorX, cursorY, kFloat_0_001_Tutorial * (float)(*(int*)&AsCTutorial(tutorial)->stepCountdown));
}

// Create the stage-completion dialog and install stage-specific camera/input behavior.
static inline void ExecuteTutorialStageEndDialog(char* tutorial, char* callbackTable) {
    TutorialCallbackWords stageEndCallback;
    char callbackScratch[0xC];
    TutorialCallbackWords stageFiveCallback;
    float dialogX;
    float dialogY;
    float messageIndexBits;
    // ABI quirk: %d writes INTEGER bits into the float-sized messageIndexBits slot;
    // reinterpret those bits when creating the dialog. This is not a float conversion.

    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialStageEnding;
    IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor), 0x1C, 1, 0, 0,
                           0);
    if (AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == 1) {
        ((CTutorial*)tutorial)->BeginStageCompletion();
        return;
    }
    sscanf(CurrentTutorialCommand(tutorial)->payload.text,
           (tutorialCommandStrings + kTutorialString_dialogIndexPositionFormat), &messageIndexBits, &dialogX, &dialogY);
    if (AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == 5) {
        TutorialDialog_CreateForMessageIndex(5, *(int*)&messageIndexBits, tutorial + kTutorialMessageDialogOffset,
                                             kFloat_0_5_Tutorial);
        void* stageFiveOwner = tutorial;
        if (tutorial != 0)
            stageFiveOwner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
        CopyTutorialTriple(stageFiveCallback, zeroInitializationWords);
        void* stageFiveDialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&stageFiveCallback, stageFiveOwner);
        TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), dialogX, dialogY);
        stageFiveDialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetCursorGroupEnabled(0, 0);
        stageFiveDialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetCursorGroupPlayerFlag(0, 0);
    } else {
        TutorialDialog_CreateForMessageIndex(5, *(int*)&messageIndexBits, tutorial + kTutorialMessageDialogOffset,
                                             kFloat_0_5_Tutorial);
        void* owner = tutorial;
        if (0 != tutorial)
            owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
        unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(
            kTutorialCallbackThisAdjustment, (callbackTable + kCallbackStageEnd), callbackScratch);
        CopyTutorialTriple(stageEndCallback, src);
        void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&stageEndCallback, owner);
        TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), dialogX, dialogY);
        dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetCursorGroupPosition(0, kTutorialDialogRightX,
                                                                       kFloat_140_TutorialDialog);
    }
    IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor), 0x53, 1, 0, 0,
                           0);
    IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_cameraActor), 0xB, 1, 0, 0, 0);
    IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor), 0x50, 1, 0, 0,
                           0);
    if (AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage != 1)
        IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor), 0x51, 1, 0,
                               0, 0);
    else
        IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor), 0x51, 0, 0,
                               0, 0);
}

// Create a fixed-position message dialog and register its continuation callback.
static inline void ExecuteTutorialMessageDialog(char* tutorial, char* callbackTable) {
    TutorialCallbackWords messageCallback;
    unsigned char callbackScratch[0xC];
    unsigned int messageIndex;
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &messageIndex);
    TutorialDialog_CreateForMessageIndex(1, messageIndex, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (tutorial)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(kTutorialCallbackThisAdjustment,
                                                                   (callbackTable + kCallbackMessage), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), kFloat_139_InGameDialog,
                               kFloat_82_InGameDialog);
}

// Create a kind-2 content layout at offsets from the tutorial dialog origin.
static inline void ExecuteTutorialMessageLayout2(char* tutorial, char* callbackTable) {
    TutorialCallbackWords messageCallback;
    float dialogOffsetX;
    float dialogOffsetY;
    float layoutWidth;
    float layoutHeight;
    char callbackScratch[0xC];
    int messageIndex;

    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    ParseTutorialDialogLayout(tutorial, &messageIndex, &dialogOffsetX, &dialogOffsetY, &layoutWidth, &layoutHeight);
    TutorialDialog_CreateForMessageIndex(1, messageIndex, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (0 != tutorial)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(kTutorialCallbackThisAdjustment,
                                                                   (callbackTable + kCallbackLayout2), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), kFloat_119_0 + dialogOffsetX,
                               (((kFloat_82_InGameDialog + dialogOffsetY))));
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->ResetAndApplyContent(2, 1, layoutWidth, layoutHeight);
}

// Create a kind-4 content layout at offsets from the tutorial dialog origin.
static inline void ExecuteTutorialMessageLayout4(char* tutorial, char* callbackTable) {
    TutorialCallbackWords messageCallback;
    float dialogOffsetX;
    unsigned char callbackScratch[0xC];
    int messageIndex;
    float dialogOffsetY;
    float layoutWidth;
    float layoutHeight;
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    ParseTutorialDialogLayout(tutorial, &messageIndex, &dialogOffsetX, &dialogOffsetY, &layoutWidth, &layoutHeight);
    TutorialDialog_CreateForMessageIndex(1, messageIndex, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (tutorial)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(kTutorialCallbackThisAdjustment,
                                                                   (callbackTable + kCallbackLayout4), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), kFloat_119_0 + dialogOffsetX,
                               ((kFloat_82_InGameDialog + dialogOffsetY)));
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->ResetAndApplyContent(4, 1, layoutWidth, layoutHeight);
}

// Create a kind-0 content layout at offsets from the tutorial dialog origin.
static inline void ExecuteTutorialMessageLayout0(char* tutorial, char* callbackTable) {
    TutorialCallbackWords messageCallback;
    float dialogOffsetX;
    unsigned char callbackScratch[0xC];
    int messageIndex;
    float layoutHeight;
    float layoutWidth;
    float dialogOffsetY;
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    ParseTutorialDialogLayout(tutorial, &messageIndex, &dialogOffsetX, &dialogOffsetY, &layoutWidth, &layoutHeight);
    TutorialDialog_CreateForMessageIndex(1, messageIndex, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (tutorial)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(kTutorialCallbackThisAdjustment,
                                                                   (callbackTable + kCallbackLayout0), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));

    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), kFloat_119_0 + dialogOffsetX,
                               (kFloat_82_InGameDialog + dialogOffsetY));
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->ResetAndApplyContent(0, 1, layoutWidth, layoutHeight);
}

// Create a pointer-relative message and compensate for right viewport overflow.

// Position the pointer-relative dialog and widen its content to compensate right-edge overflow.
static inline void PositionTutorialPointerDialog(char* tutorial, void*& dialog, float& rightOverflow, float& dialogTop,
                                                 float& positiveOverflow) {
    rightOverflow = AsCTutorial(tutorial)->messagePosition[0] - kFloat_22_TextWidget +
                    ((CInGameTutorialDialog*)dialog)->tutorialWidth - ((WView*)g_view)->viewportWidth;
    dialogTop = AsCTutorial(tutorial)->messagePosition[1] - kFloat_128_TextWidget;
    positiveOverflow = kFloat_0_Tutorial;
    if (rightOverflow > positiveOverflow)
        positiveOverflow = rightOverflow;
    TutorialDialog_SetPosition(
        dialog, AsCTutorial(tutorial)->messagePosition[0] - kFloat_22_TextWidget - positiveOverflow, dialogTop);
    rightOverflow = rightOverflow > kFloat_0_Tutorial ? rightOverflow : kFloat_0_Tutorial;
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->ResetAndApplyContent(2, 1, kFloat_22_TextWidget + rightOverflow,
                                                                 kFloat_128_TextWidget);
}

// Create a pointer-relative message, retaining its callback and constraining it to the viewport.
static inline void ExecuteTutorialMessageAtPointer(char* tutorial, char* callbackTable, float& rightOverflow,
                                                   float& dialogTop, float& positiveOverflow) {
    TutorialCallbackWords messageCallback;
    char callbackScratch[0xC];
    unsigned int messageIndex;
    // Keep dialog's right edge within viewport: subtract positive overflow
    // from x, then widen layout content by the same overflow. UI pixel units.
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &messageIndex);
    TutorialDialog_CreateForMessageIndex(1, messageIndex, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (0 != tutorial)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(
        kTutorialCallbackThisAdjustment, (callbackTable + kCallbackAtPointer), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    PositionTutorialPointerDialog(tutorial, dialog, rightOverflow, dialogTop, positiveOverflow);
}

// Parse dialog kind and size, then center its rectangle in UI pixels.
static inline void ExecuteTutorialSizedMessageDialog(char* tutorial, char* callbackTable) {
    float left;
    float dialogRect[4];
    float top;
    float width;
    float height;
    TutorialCallbackWords messageCallback;
    char callbackScratch[0xC];
    int messageIndex;
    unsigned int dialogKind;
    float dialogWidth;
    float dialogHeight;

    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text,
           (tutorialCommandStrings + kTutorialString_dialogIndexSizeKindFormat), &messageIndex, &dialogWidth,
           &dialogHeight, &dialogKind);
    TutorialDialog_CreateForMessageIndex(dialogKind, messageIndex, tutorial + kTutorialMessageDialogOffset,
                                         kFloat_0_5_Tutorial);
    void* owner = tutorial;
    if (tutorial != 0)
        owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
    unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(
        kTutorialCallbackThisAdjustment, (callbackTable + kCallbackSizedMessage), callbackScratch);
    CopyTutorialTriple(messageCallback, src);
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&messageCallback, owner);
    height = dialogHeight;
    width = dialogWidth;

    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    top = kFloat_82_InGameDialog;
    dialogRect[1] = top;
    left = kFloat_0_5_Tutorial * (((WView*)g_view)->viewportWidth - width);
    dialogRect[2] = width;
    ((CInGameTutorialDialog*)dialog)->positionX = left;
    ((CInGameTutorialDialog*)dialog)->positionY = top;
    ((CInGameTutorialDialog*)dialog)->tutorialWidth = width;
    dialogRect[3] = height;
    dialogRect[0] = left;
    ((CInGameTutorialDialog*)dialog)->tutorialHeight = height;
}

// Resolve a named world point and project the tutorial marker onto course ground.
static inline void ExecuteTutorialTargetNamedPoint(char* tutorial) {
    TutorialPositionWords pointPositionWords;
    WVector groundPosition;
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialStageEnding;

    (*(char*)&AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialMarkerVisible) = 1;
    char* namedPoint =
        (char*)CourseData_FindNamedPoint(AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData,
                                         CurrentTutorialCommand(tutorial)->payload.text);
    CopyTutorialPositionWords(pointPositionWords, ((TutorialNamedPoint*)namedPoint)->positionWords);
    char* golfDocBytes = (*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* quadTree = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPosition, quadTree, &pointPositionWords, 0, 0, 1);
    *(WVector*)(golfDocBytes + 0xAC) = groundPosition;
}

// Parse the receiver name and integer payload, then dispatch synchronously.
static inline void ExecuteTutorialDispatchActorMessage(char* tutorial) {
    int messageCode;
    int messageValue3;
    unsigned int messageValue1;
    char name[0x20];
    int messageValue2;
    messageValue2 = 0;
    messageValue1 = 0;
    messageValue3 = 0;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text,
           (tutorialCommandStrings + kTutorialString_actorMessageFormat), name, &messageCode, &messageValue1,
           &messageValue2, &messageValue3);
    IActor_DispatchMessage(tutorial, tutorial, name, messageCode, messageValue1, messageValue2, messageValue3, 0);
}

// Consume integer text without applying its value.
static inline void ExecuteTutorialParseInteger(char* tutorial) {
    unsigned int ignoredValue;
    // Consume syntactically valid integer text without applying its value.
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &ignoredValue);
}

// Block primary actions and optionally suppress pointer input; keep dialog actions available.
static inline void ExecuteTutorialBlockInput(char* tutorial) {
    int blockPointerInput;
    // Block primary action slots; text additionally chooses pointer suppression.
    blockPointerInput = 0;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &blockPointerInput);
    int shouldBlockPointer = ((0 != blockPointerInput));
    int* inputWords = SetTutorialActionBlocking(1);

    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 1;
    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction19] = 0;
    if ((shouldBlockPointer)) {
        Pointer_SuppressInput(g_inputPointer);
        return;
    }
    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction24] = 0;
    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction17] = 0;
    Pointer_ClearInputSuppression(g_inputPointer);
}

// Restore pointer and primary input while retaining the blocked action-20 policy.
static inline void ExecuteTutorialAllowInput(char* tutorial) {
    Pointer_ClearInputSuppression(g_inputPointer);
    int* inputWords = SetTutorialActionBlocking(0);
    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 0;
    inputWords = GetTutorialInputManagerWords(inputWords);
    ((GameInputManager*)inputWords)->actionBlocked[kTutorialBlockedAction20] = 1;
}

// Show the lesson prompt with a continuation callback, or resume the command stream.
static inline void ExecuteTutorialLessonPrompt(char* tutorial, char* callbackTable) {
    TutorialCallbackWords lessonCallback;
    unsigned char callbackScratch[0xC];
    int showPrompt;
    showPrompt = 0;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &showPrompt);
    if (0 != showPrompt) {
        int* previous = (int*)((int&)GetInputManagerSlot());
        (*(int*)&AsCTutorial(tutorial)->state) = kTutorialWaitingForDialog;
        int* inputWords = SetTutorialActionBlocking(1, previous);
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 1;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction19] = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction24] = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction17] = 0;
        Pointer_ClearInputSuppression(g_inputPointer);
        TutorialDialog_CreateForMessageIndex(6, 0x1E, tutorial + kTutorialMessageDialogOffset, kFloat_0_5_Tutorial);
        void* owner = tutorial;
        if (tutorial)
            owner = ((char*)&AsCTutorial(tutorial)->secondaryVtable);
        unsigned int* src = (unsigned int*)Runtime_AdjustMemberPointer(
            kTutorialCallbackThisAdjustment, (callbackTable + kCallbackLessonPrompt), callbackScratch);
        CopyTutorialTriple(lessonCallback, src);
        void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetResultCallback(&lessonCallback, owner);
        dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        TutorialDialog_SetPosition((*(void**)(tutorial + kTutorialMessageDialogOffset)), kFloat_119_0,
                                   kFloat_82_InGameDialog);
        CurrentTutorialMessageDialog(tutorial)->SetCursorGroupPosition(0, kTutorialDialogRightX,
                                                                       kFloat_341_TutorialDialog);
        dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
        CurrentTutorialMessageDialog(tutorial)->SetCursorGroupPosition(1, kTutorialDialogRightX,
                                                                       kFloat_390_TutorialDialog);
        ((CMouseCursor*)g_mouseCursor)->moveToActive = 0;
    } else {
        (*(int*)&AsCTutorial(tutorial)->state) = kTutorialRunningCommands;
        IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_golfRuleActor), 0x23, 0, 0,
                               0, 0);
    }
}

// Enter aim alignment with its input policy, or reset shot panels and resume commands.
static inline void ExecuteTutorialAimAlignment(char* tutorial) {
    int enableAimAlignment;
    enableAimAlignment = 0;
    sscanf(CurrentTutorialCommand(tutorial)->payload.text, (tutorialCommandStrings + kTutorialString_integerFormat),
           &enableAimAlignment);
    if (0 != enableAimAlignment) {
        (*(int*)&AsCTutorial(tutorial)->state) = kTutorialAimAlignment;
        IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_golfRuleActor), 0x25, 0, 0,
                               0, 0);
        Pointer_ClearInputSuppression(g_inputPointer);
        int* inputWords = SetTutorialActionBlocking(0);
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialScriptAction6] = 1;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialBlockedAction20] = 1;
    } else {
        IActor_DispatchMessage(tutorial, tutorial, (tutorialCommandStrings + kTutorialString_screenActor),
                               kScreenResetShotPanels, 0, 0, 0, 0);
        (*(int*)&AsCTutorial(tutorial)->state) = kTutorialRunningCommands;
    }
}

// Close and release the retained dialog, deleting only when its reference count reaches zero.
static inline void ExecuteTutorialCloseDialog(char* tutorial) {
    // Close first, then drop tutorial's reference; delete only at refcount zero.
    void* dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    CurrentTutorialMessageDialog(tutorial)->SetDialogState(0);
    dialog = (*(void**)(tutorial + kTutorialMessageDialogOffset));
    if (dialog) {
        if (--((TutorialDialogReferenceCount*)dialog)->references == 0 && dialog != 0)
            ((TutorialDialogInterface*)dialog)->Destroy(1);
        (*(int*)&(*(void**)(tutorial + kTutorialMessageDialogOffset))) = 0;
    }
}

// Copy the current save record, mark tutorial completion, and publish the updated record.
static inline void ExecuteTutorialSetSaveFlag(char* tutorial) {
    unsigned char updatedSaveRecord[0x828];
    // Copy current save record, set completion bit, and publish the updated copy.

    if (!((int&)GetSaveDataSlot())) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRecord_CopyCtor(updatedSaveRecord, SaveData_GetCurrentRecord(((int&)GetSaveDataSlot())));
    ((TutorialSaveRecord*)updatedSaveRecord)->flags |= kTutorialSaveCompletionFlag;
    if (!((int&)GetSaveDataSlot())) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetCurrentRecord(((int&)GetSaveDataSlot()), updatedSaveRecord);
}

#pragma pop

#pragma push
#pragma always_inline on
#pragma inline_depth(32)
// Consume one record and retire full blocks only after retaining the consumed-prefix window.
// The queue owns its blocks; payloads from the consumed record must no longer be borrowed.
static inline void ConsumeTutorialCommand(char* tutorial) {
    (*(int*)&AsCTutorial(tutorial)->pendingCommandCount) -= 1;
    (*(unsigned int*)&AsCTutorial(tutorial)->commandReadOffset) += 1;
    while ((*(unsigned int*)&AsCTutorial(tutorial)->commandReadOffset) >= kRetainedCommandBlocks * kCommandsPerBlock) {
        appFreeUni__FPv((((void**)AsCTutorial(tutorial)
                              ->commandBlocks))[(*(unsigned int*)&AsCTutorial(tutorial)->firstCommandBlockIndex)]);
        (*(unsigned int*)&AsCTutorial(tutorial)->firstCommandBlockIndex) += 1;
        if ((*(unsigned int*)&AsCTutorial(tutorial)->firstCommandBlockIndex) ==
            (*(unsigned int*)&AsCTutorial(tutorial)->commandBlockCapacity))
            (*(unsigned int*)&AsCTutorial(tutorial)->firstCommandBlockIndex) = 0;
        AsCTutorial(tutorial)->commandBlockCount -= 1;
        (*(unsigned int*)&AsCTutorial(tutorial)->commandReadOffset) -= kCommandsPerBlock;
    }
    if ((*(int*)&AsCTutorial(tutorial)->pendingCommandCount) == 0)
        (*(unsigned int*)&AsCTutorial(tutorial)->commandReadOffset) = kTutorialEmptyReadOffset;
}

// Reconcile input with the resulting dialog/stage/aim state before zero-delay recursion.
// RunningCommands deliberately preserves the policy installed by its opcode.
static inline void ApplyTutorialCommandStateInputPolicy(char* tutorial) {
    int state = (*(int*)&AsCTutorial(tutorial)->state);
    if (state == kTutorialWaitingForDialog) {
        int* inputWords = SetTutorialActionBlocking(1);
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 1;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction19] = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction24] = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialDialogAction17] = 0;
        Pointer_ClearInputSuppression(g_inputPointer);
        ((CMouseCursor*)g_mouseCursor)->moveToActive = 0;
    } else if (state == kTutorialStageEnding) {
        Pointer_ClearInputSuppression(g_inputPointer);
        int* inputWords = SetTutorialActionBlocking(0);
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialBlockedAction20] = 1;
    } else if (state == kTutorialAimAlignment) {
        Pointer_ClearInputSuppression(g_inputPointer);
        int* inputWords = SetTutorialActionBlocking(0);
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->selectedPadLookupDisabled = 0;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialScriptAction6] = 1;
        inputWords = GetTutorialInputManagerWords(inputWords);
        ((GameInputManager*)inputWords)->actionBlocked[kTutorialBlockedAction20] = 1;
    }
}
#pragma pop

// Execute the next tutorial opcode, release consumed queue blocks and update input suppression.
#pragma push
#pragma inline_depth(32)
void CTutorial_ExecuteCurrentCommand(void* object) {
    // Caller supplies a live tutorial and parseable command payloads. sscanf
    // results are intentionally unchecked. This may execute several zero-delay
    // successors recursively; it does not promise exactly one command per call.
    // Helpers own parser scratch and register callbacks synchronously. Dialog handles
    // persist on tutorial. Local declaration order and caller float scratch retain Wii code.
    // Phase 1: gate on queue/dialog state and load the current record's delay.
    char* callbackTable = tutorialMessageIndices;
    char* tutorial = (char*)object;
    if ((*(int*)&AsCTutorial(tutorial)->pendingCommandCount) == 0 ||
        (*(int*)&AsCTutorial(tutorial)->modeMessageDialog) != 0)
        return;
    (*(int*)&AsCTutorial(tutorial)->state) = kTutorialRunningCommands;
    (*(int*)&AsCTutorial(tutorial)->stepCountdown) = CurrentTutorialCommand(tutorial)->delayMilliseconds;

    // Phase 2: apply one opcode. Singleton access order and virtual calls are observable.
    switch ((CurrentTutorialCommand(tutorial)->opcode)) {
    // Input opcodes: action byte -> pending press, with optional retained forcing.
    case kCommandPulseAction:
        // Pulse the current signed action-slot index once.
        (*(int*)((char*)GetTutorialInputManagerWords() +
                 (kTutorialPendingActionsOffset + CurrentTutorialCommand(tutorial)->payload.action * 4))) = 1;
        break;
    case kCommandForceAction:
        // Retain the forced action index and pulse its pending-input word.
        ExecuteTutorialForceAction(tutorial);
        break;
    case kCommandClearForcedAction:
        // Stop retaining an action between update calls.
        (*(int*)&AsCTutorial(tutorial)->forcedActionIndex) = -1;
        break;
    case kCommandPulseActionAlternate:
        // Alternate opcode shares the same one-pulse action protocol.
        (*(int*)((char*)GetTutorialInputManagerWords() +
                 (kTutorialPendingActionsOffset + CurrentTutorialCommand(tutorial)->payload.action * 4))) = 1;
        break;
    case kCommandMoveCursor:
        // Parse UI coordinates and start a cursor move lasting the command delay in milliseconds.
        ExecuteTutorialMoveCursor(tutorial);
        break;
    // Dialog family: parse payload -> create retained handle -> adjust callback
    // ABI -> set UI geometry. Each case selects its own callback-table entry.
    // Callback scratch is local; preserve the registration call before it expires.
    case kCommandStageEndDialog:
        // Create the stage-completion dialog and install stage-specific camera/input behavior.
        ExecuteTutorialStageEndDialog(tutorial, callbackTable);
        break;
    case kCommandMessageDialog:
        // Create a fixed-position message dialog and register its continuation callback.
        ExecuteTutorialMessageDialog(tutorial, callbackTable);
        break;
    case kCommandMessageLayout2:
        // Create a kind-2 content layout at offsets from the tutorial dialog origin.
        ExecuteTutorialMessageLayout2(tutorial, callbackTable);
        break;
    case kCommandMessageLayout4:
        // Create a kind-4 content layout at offsets from the tutorial dialog origin.
        ExecuteTutorialMessageLayout4(tutorial, callbackTable);
        break;
    case kCommandMessageLayout0:
        // Create a kind-0 content layout at offsets from the tutorial dialog origin.
        ExecuteTutorialMessageLayout0(tutorial, callbackTable);
        break;
    case kCommandMessageAtPointer: {
        // Create a pointer-relative message and constrain it to the viewport.
        float rightOverflow, dialogTop, positiveOverflow;
        ExecuteTutorialMessageAtPointer(tutorial, callbackTable, rightOverflow, dialogTop, positiveOverflow);
        break;
    }
    case kCommandSizedMessageDialog:
        // Parse dialog kind and size, then center its rectangle in UI pixels.
        ExecuteTutorialSizedMessageDialog(tutorial, callbackTable);
        break;
    // Named-point coordinates are world space; project them onto course ground.
    case kCommandTargetNamedPoint:
        // Resolve a named world point and project the tutorial marker onto course ground.
        ExecuteTutorialTargetNamedPoint(tutorial);
        break;
    // Actor/message domains are external: this opcode only parses and forwards words.
    case kCommandDispatchActorMessage:
        // Parse the receiver name and integer payload, then dispatch synchronously.
        ExecuteTutorialDispatchActorMessage(tutorial);
        break;
    case kCommandParseInteger:
        // Consume integer text without applying its value.
        ExecuteTutorialParseInteger(tutorial);
        break;
    case kCommandBlockInput:
        // Block primary actions and optionally suppress pointer input; keep dialog actions available.
        ExecuteTutorialBlockInput(tutorial);
        break;
    case kCommandAllowInput:
        // Restore pointer and primary input while retaining the blocked action-20 policy.
        ExecuteTutorialAllowInput(tutorial);
        break;
    case kCommandLessonPrompt:
        // Show the lesson prompt with a continuation callback, or resume the command stream.
        ExecuteTutorialLessonPrompt(tutorial, callbackTable);
        break;
    case kCommandAimAlignment:
        // Enter aim alignment with its input policy, or reset shot panels and resume commands.
        ExecuteTutorialAimAlignment(tutorial);
        break;
    case kCommandCloseDialog:
        // Close and release the retained dialog, deleting only when its reference count reaches zero.
        ExecuteTutorialCloseDialog(tutorial);
        break;
    case kCommandSetSaveFlag:
        // Copy the current save record, mark tutorial completion, and publish the updated record.
        ExecuteTutorialSetSaveFlag(tutorial);
        break;
    case kCommandFinishCursorMove:
        // Clear the cursor movement latch without changing position.
        ((CMouseCursor*)g_mouseCursor)->moveToActive = 0;
        break;
    default:
        break;
    }
    // Phase 3: consume the command; free whole blocks only after retaining a consumed block.
    ConsumeTutorialCommand(tutorial);
    int mode = AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
    if (mode == 5 && (*(int*)&AsCTutorial(tutorial)->pendingCommandCount) == 0)
        return;
    if (mode == 2 && AsCSharedDoc(g_sharedDoc)->gameMode == 4 &&
        (*(int*)&AsCTutorial(tutorial)->pendingCommandCount) == 0)
        return;
    // Phase 4: reconcile input policy with dialog/stage/aim state, then run zero-delay successors.
    ApplyTutorialCommandStateInputPolicy(tutorial);
    if ((*(int*)&AsCTutorial(tutorial)->stepCountdown) == 0)
        CTutorial_ExecuteCurrentCommand(tutorial);
}

#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621E8", _ctors_u80209C3C_0, Tutorial_InitializeTwoPi);
#pragma pop
