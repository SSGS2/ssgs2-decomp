// Story (Conte) event script interpreter: the command handlers for models, faces, effects,
// messages, sound, flags, the tour hint and versus windows, and tour conditions.
// The callback queues and keyed maps that the commands wait on are defined here too.

#include <decomp/ct_member.h>
#include "MSL/error_message.h"
// Event/Conte script interpreter: story commands, facial/model/effect events,
// and shared-file loading. Command buffers and loaded data are borrowed; queues
// and keyed maps have separate ownership described at the handlers below.
// For a story, model, sound or loading mod, start with the navigation and wire
// layout index, then the selected handler's completion/failure contract.
#include "classes/CEventModelSC_MDL_MOT.h"
#include "classes/CEventEffect.h"
#include "classes/EventScriptByteTreePair.h"
#include "classes/CMsgDraw.h"
#include "classes/EventScriptByteTreeNode.h"
#include "classes/ScriptOwnedBuffer80092804.h"
#include "classes/EventScriptCommandView.h"
#include "classes/CSharedDoc.h"
#include "classes/CEventHandle.h"
#include "classes/EventScriptStateView.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char bpPlayerResourceStrings[];
}
}

#include "game/string.h"

#include "classes/MWStringStorage.h"
#include "classes/CPangyaCommonDialog.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CTourVSWindow.h"
#include "classes/TourVSWindowData.h"

// Navigation (search the exact function name; helpers for a handler precede it):
// - ScriptSharedFiles_ResetRequiredFilesAndHandlers: shared-file reset/registration.
// - ProcessEventSkipCommand .. ProcessFacePartsCommand: dialogs, messages, tour
//   progress, appearance conditions and character-slot commands.
// - ScriptState_GetSharedFileList .. ProcessSetAppearRoadInfoCommand: shared-file
//   access, hint/VS windows, branches, variable binding, waits and tour flags.
// - ExecuteFaceSubscript: four-level offset-table lookup and a nested interpreter.
// - ExecuteAttachedEffectCommand / ProcessModelMoveCommand: effect/keyed-model maps.
// - ProcessSoundBgmStopCommand .. ProcessSoundSeStopCommand: sound callback readers.
// - ScriptSharedFiles_RemoveRequiredFilesAndRelease: required-file ownership teardown.
// - ProcessModelHeadCommand .. ProcessObjectDrawCommand: model/message/effect readers.
// - StringVector_AppendCopy: deep string append; CEventHandle's constructor: full offset map.
// - EventScript_RunAndSweep / EventScript_SweepPendingCallbacks: execution and polling.
// - ProcessModelMotionCommand: padded-name motion reader between head reset and move.
// All Process*/Execute* names in this map use the CEventConteProc_ prefix unless
// written in full. Source order follows recovered functions, not opcode order.
//
// To change a command: read its short purpose/operand contract, then the event
// constructor it invokes. The wrapper constructor only captures the event and
// copies diagnostic names. Capacity arithmetic below it implements queue storage;
// EventScript_RunAndSweep performs later callback polling. Preserve each handler's
// skip length, synchronous wait path, async destination, and pending-pointer clear.
// Queue append ownership does NOT imply exception-safe transfer before publication.

// Command wire-layout reference (offsets/lengths below are BYTES):
// Common header: +0 opcode, +1 byte operand, +2 halfword operand. Commands start
// on 4-byte boundaries. Multibyte operands in this unit use native PPC loads;
// do not copy the separate GOTO handler's little-endian swap into these readers.
// A variable string is NUL-terminated; skip (strlen+4)&~3 bytes INCLUDING its
// terminator/padding. E.g. lengths 0..3 occupy 4 bytes, length 4 occupies 8.
// The script buffer and command slices are borrowed and must stay valid while
// an event is pending. No bounds, terminator, or alignment validation is added.
//
// Reader                                  Observed payload / total command span
// ProcessModelMoveCommand                  28 bytes; +8 key, +0x19 async flag;
//                                         other payload words go to its constructor.
// ProcessModelMotionCommand                8-byte prefix, padded name at +8,
//                                         then a 4-byte trailer: byte 0 is async.
// ProcessSoundBgmStopCommand               8 bytes; payload passed to stop event.
// Script_FadeTourHintWindow                4 bytes; +1: 0=in, nonzero=out;
//                                         +2: unsigned duration in update ticks.
// Script_StartTourVSWindowTransition       4 bytes, no operand read in this handler.
// Additional command-specific operands are described at their readers; this
// table states only verified offsets, not a guessed global opcode schema.
//
// Commands return 1 after advancing the cursor, or 0 to resume on a later tick.
// Condition handlers still run with nesting bit 0 set; other active bits skip them.
enum ScriptConstants {
    kOuterConditionMask = 0xFFFE,
    kBasicCommandBytes = 4,
    kOneWordCommandBytes = 8,
    kTwoWordCommandBytes = 12,
    kModelMoveCommandBytes = 28,
    kScriptSlotCount = 256,
    kSharedFileEndOpcode = 0x0A,
    kDialogYes = 1,
    kDialogNo = 2,
    kYesNoDialogKind = 2,
    kDialogInputDelay = 10,
    kDialogLeft = 200,
    kDialogTop = 100,
    kWindowLoadRequested = 1,
    kWindowReady = 2,
    kFullOpacity = 255,
    kTourVSFadeOutStep = 32,
    kFadeInCommand = 0,
    kProgressRows = 11,
    kProgressEntriesPerRow = 35,
    kActorSlotCount = 5,
    kRoadInactive = 0,
    kRoadReadyToReveal = 2,
    kRoadRevealInProgress = 3,
    kBlockPointInProgress = 5,
    kBlockPointFinished = 4,
    kTourClearedBit = 5,
    kPointBlockedBit = 13,
    kRoadBaseVariantSeenBit = 5,
    kRoadHundredsVariantSeenBit = 7
};
// Guarded event/queue tick increments stop here; yes/no dialog and block-point
// command ticks increment without a saturation guard.
static const unsigned int kSaturatedCommandTick = 0xFFFFFFFFu;
// Byte-key tree cardinality guard, independent of event tick saturation.
static const unsigned int kScriptByteTreeCountLimit = 0xFFFFFFFFu;
enum ScriptEventOperands {
    kModelMoveKeyOffset = 8,
    kModelMoveAsyncOffset = 0x19,
    kMotionNameOffset = 8,
    kMotionTrailerBytes = 4,
    kBgmVolumeCommandBytes = 12,
    kBgmPitchCommandBytes = 12,
    kSePitchNameOffset = 12
};
static const unsigned int kMaxPointerVectorSize = 0x3FFFFFFFu;
static const unsigned int kMaxStringVectorSize = 0x15555555u; // Maximum count of 12-byte strings.
static const unsigned int kPointerVectorSmallGrowthLimit = 0x15555555u;
// Twelve-byte string records have a different maximum count from pointer vectors.
static const unsigned int kStringVectorSmallGrowthLimit = 0x071C71C7u;
static const unsigned int kStringVectorLargeGrowthLimit = 0x0E38E38Eu;
static const unsigned int kPointerVectorLargeGrowthLimit = 0x2AAAAAAAu;
static const unsigned int kMaxCommandRecordVectorSize = 0x1FFFFFFFu;
static const unsigned int kRecordVectorSmallGrowthLimit = 0x0AAAAAAAu;
static const unsigned int kRecordVectorLargeGrowthLimit = 0x15555554u;

// These byte layouts describe subobjects; they do not construct or own storage.
// Reserved spans are unmodeled by that view and may contain other live fields.
struct ScriptManagerView {
    char reserved_000[0x24];
    short startPoint;
    unsigned char characterMoveMode;
    char reserved_027;
    short characterMoveTarget;
    char reserved_02A[2];
    unsigned int eventSkip;
    void** dialogSlot;
    void* tourState;
};
struct ScriptProgressRow {
    short reserved_000;
    short selectedEntry;
    unsigned short entryFlags[kProgressEntriesPerRow];
};
struct ScriptSaveProgressView {
    char reserved_000[0x4EC];
    ScriptProgressRow progress[kProgressRows];
    signed char selectedRowByLane[8];
    signed char selectedLane;
    char reserved_823;
    unsigned short coinCount;
    unsigned char tourFlags;
    char reserved_827;
};
struct ScriptTourStateView {
    ScriptSaveProgressView* record;
    unsigned char requestState;
    char reserved_005[3];
    int requestedEntry;
    unsigned char requestComplete;
    char reserved_00D[3];
    unsigned int elapsedTicks;
    unsigned char blockState;
    char reserved_015[3];
    unsigned int blockPoint;
    char reserved_01C[4];
    unsigned int blockTicks;
    char reserved_024[0x10];
    unsigned int exitRequested;
};
struct ScriptStringVectorView {
    MWStringStorage* data;
    unsigned int size;
    unsigned int capacity;
};

// Repeated implementation patterns used below:
// Singleton blocks obtain/publish existing shared objects and register their
// destructors once. They are access scaffolding; the call AFTER the block is
// the command's side effect. Repeated checks are intentionally kept in place.
// Async wrappers own a pending callback and copies of diagnostic event/opcode
// names. A queue/map takes the wrapper only when its pointer is installed; the
// interpreter then clears pending. A synchronous event stays interpreter-owned
// until update reports completion and the event is destroyed.
// Pointer-vector growth: reserve -> write new tail -> copy old handles -> zero
// old slots -> swap storage/count/capacity -> guard frees old array. Handles are
// transferred, not cloned. String append deep-copies its input; growth either
// copy-constructs old elements or transfers their representations with zeroing,
// depending on the specialization. A zeroed old string releases no heap data.
// Common link prefix for all byte-key trees; full node/pair schema is below.
struct ScriptTreeLinks {
    ScriptTreeLinks* left;
    ScriptTreeLinks* right;
};
struct ScriptByteMapView {
    unsigned int size;
    void* root;
    void* begin;
};
struct TourHintFadeView {
    char reserved_000[0x38];
    char fadeControl[4];
    unsigned char opacity;
};
struct ScriptOwnedEventView {
    char reserved_000[0x20];
    void** vtable;
};
// Character data starts at +1 within the 12-byte MSL small-string storage.
struct ScriptPrefixView {
    char reserved_000[0x409];
    char inlinePrefix[11];
};
struct ScriptEffectMapView {
    char reserved_000[0x47C];
    char effectMap[0x10];
};
struct ScriptObjectMapView {
    char reserved_000[0x470];
    ScriptByteMapView objects;
};
struct ScriptResourceMapView {
    char reserved_000[0x490];
    ScriptByteMapView resources;
};
// This span crosses separate BSS globals; retain the original relocation anchor.
struct SharedFileCleanupView {
    char reserved_000[0x48];
    ScriptByteMapView handlers;
};
struct ActorSlotView {
    unsigned char characterIndex;
    char reserved_001[3];
    char* puppet;
};

// Offsets retain the original addition order where CodeWarrior schedules it differently.
#define SCRIPT_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
#define SCRIPT_VIEW_CHECK(type, member, offset) \
    typedef char type##_##member##_offset[((int)&((type*)0)->member == (offset)) ? 1 : -1]
SCRIPT_VIEW_CHECK(ScriptManagerView, tourState, 0x34);
SCRIPT_VIEW_CHECK(ScriptSaveProgressView, progress, 0x4EC);
SCRIPT_VIEW_CHECK(ScriptSaveProgressView, selectedRowByLane, 0x81A);
SCRIPT_VIEW_CHECK(ScriptSaveProgressView, selectedLane, 0x822);
SCRIPT_VIEW_CHECK(ScriptSaveProgressView, coinCount, 0x824);
SCRIPT_VIEW_CHECK(ScriptSaveProgressView, tourFlags, 0x826);
SCRIPT_VIEW_CHECK(ScriptTourStateView, exitRequested, 0x34);
SCRIPT_VIEW_CHECK(TourHintFadeView, opacity, 0x3C);
#undef SCRIPT_VIEW_CHECK

// One pool object, including the alignment gap at 0x5E; no strings are copied.
struct ScriptResourceStrings {
    char buttonClick[0x13];
    char vectorLengthError[0x14];
    char lengthErrorTypeName[0x37];
    char reserved_05E;
    char voiceExtension[5];
    char facialData[0x0B];
    char treeLengthError[0x1A];
};

#include "game/singletons.h"
extern "C" {
// Leave the iterator storage untouched (the empty iterator constructor).
void ScriptMapIterator_DefaultConstruct();

extern void ScriptByteMap_FreeSubtree(void*, void*);
extern void appFreeUni__FPv(void*);
extern void* EventFxMap_FindInsertionPosition(void*, unsigned char*, void**, unsigned char*, unsigned char*);
extern void* EventFxMap_InsertNode(void*, unsigned int, unsigned int, unsigned int, void*);
extern unsigned char scriptByteHandlerBinding_17;
extern unsigned int s_scriptByteMapDefaultValue;
extern char s_scriptByteKeyMap[];
extern unsigned int s_storyConteRequiredFilesStorage[];

// Unrolled tree frees and capture lifetimes preserve the original register allocation.
// Reset the file-command handler map, register handler 17, and destroy required-file names.
void ScriptSharedFiles_ResetRequiredFilesAndHandlers();

// One interpreter receiver for these command handlers; manager is borrowed
// at +0x4A0, with no event/tree ownership. elapsedCommandTicks is the per-command
// tick word at +0x41C (reset by dispatch).
// Message requests use a different view below: their root is pending at +0x42C.
struct ScriptCommandContextView {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[2];
    unsigned char conditionCount, trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
    unsigned int elapsedCommandTicks;
    char reserved_420[0x80];
    void* manager;
};

// TA_EVENT_SKIP: clear the manager event-skip field and consume the command.

struct Fn8009B35CObject {
    char reserved_0[0x30];
    void** dialogSlot;
};
extern void __ct__19CPangyaCommonDialogFPvi(void*, int, int);
struct Fn8009B35CNode {
    char opaqueStorage[0x44];
    // Construct the owned common dialog with the supplied constructor operands.
    Fn8009B35CNode(int a, int b) {
        __ct__19CPangyaCommonDialogFPvi(this, a, b);
    }
};
extern char s_eventScriptEmptyDialogText[];

// Show a yes/no dialog; wait for a result and count Yes as a true condition.

struct Fn8009BBFCRoot {
    char reserved_0[0x34];
    void** slot;
};
struct Fn8009BBFCString {
    unsigned char data[12];
};
extern int String_ReplaceRange(Fn8009BBFCString*, unsigned int, unsigned int, const char*, const char**);
extern void String_ReserveCapacity(Fn8009BBFCString*, unsigned int);

extern void Character_GetAssetNameSuffix(Fn8009BBFCString*, signed char);
extern void Event_GetAssetNameSuffix(Fn8009BBFCString*, int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __ct__23CEventMessageSC_MSG_REQFPUc(void*, void*);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void __dt__9CBpPlayerFv();
extern char s_bpPlayer[], s_bpPlayerDtorRegistration_Exhibition[];
extern void __ct__9CBpPlayerFv(void*);
extern void __register_global_object(void*, void (*)(), void*);
extern signed char s_bpPlayerInitialized;
struct Fn80095808Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int index;
    char reserved_420[0x0C];
    void* root;
    char reserved_430[0x60];
    unsigned int enabled;
    void* tree;
};
struct Fn80095808Node {
    Fn80095808Node* left;
    Fn80095808Node* right;
    char reserved_8[4];
    unsigned char key;
    char reserved_D[3];
    void* payload;
};
struct Fn80095808Base {
    virtual void destroy(int) = 0;
    virtual int update(unsigned int) = 0;
};
struct Fn80095808Root {
    char opaqueStorage[0x24];
    // Construct the encoded command event in this owned storage.
    Fn80095808Root(unsigned char* p) {
        __ct__23CEventMessageSC_MSG_REQFPUc(this, p);
    }
};
// Run a message request, synchronizing face channels with text reveal until completion.
#define __register_global_object SDK_register_global_object
#define __throw SDK_throw
#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/CEventConteProc.h"
#include "classes/CBpPlayer.h"
#include "classes/CTourMessage.h"
#include "classes/CFade.h"
#include "classes/CFlashMemoryMenu.h"
#include "classes/CEventFace.h"
#include "classes/CEventHandleTreeIterator.h"
#include "classes/CMessageOwnerLayout.h"
#include "classes/CTourHintWindow.h"
#undef __throw
#undef __register_global_object
extern char s_tourVoiceDecideSuffix[8], s_tourVoiceWinSuffix[5], s_tourVoiceLoseSuffix[6], s_tourVoiceChangeSuffix[8],
    s_tourVoiceItemSuffix[6], s_tourVoiceVersusSuffix[4];
extern char tourVoiceResourceName[];
extern char s_tourVoiceKenSuffix[5], s_tourVoiceErikaSuffix[7], s_tourVoiceDaisukeSuffix[], s_tourVoiceCecilliaSuffix[];
extern char s_tourVoiceMaxSuffix[5], s_tourVoiceKoohSuffix[6], s_tourVoiceArinSuffix[6], s_tourVoiceKazSuffix[5];
extern unsigned int g_soundManager;
// Original stored sound parameters: 0.5, 200.0, and 1.0, respectively.
extern float kFloat_0_5_EventScriptSoundMinimumGain, kFloat_200_EventScriptSoundRange, kFloat_1_TourWindow;
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

// Preserve MSL allocator-tag copies and exception boundaries; their captures affect matching.
struct Fn8009DB08Tag {
    volatile unsigned char value;
    char pad[3];
    // Leave the allocator tag byte uninitialized, as required by this ABI expansion.
    Fn8009DB08Tag() {}
    // Copy the allocator tag byte used by MSL string construction.
    Fn8009DB08Tag(const Fn8009DB08Tag& other) : value(other.value) {}
};
// Physical diagnostic-string storage; the third word also holds inline characters.
struct ScriptDiagnosticWords128 {
    unsigned int tag, length, heapOrInlineTail;
};

// Read a diagnostic string's byte length, preserving the volatile tag-word load.
static inline unsigned int ScriptDiagnosticLength128(void* storage) {
    volatile unsigned int* p = &((ScriptDiagnosticWords128*)storage)->tag;
    return (*p >> 31) == 0 ? (*(unsigned char*)p & 0x7f) : ((ScriptDiagnosticWords128*)storage)->length;
}

// Initialize one diagnostic string as empty inline storage without allocating.
static inline void ClearScriptDiagnosticString128(void* storage) {
    ((ScriptDiagnosticWords128*)storage)->tag = 0;
    ((ScriptDiagnosticWords128*)storage)->length = 0;
    ((ScriptDiagnosticWords128*)storage)->heapOrInlineTail = 0;
}

// Adapt borrowed string bytes/tag to the engine range-replacement ABI at one named boundary.
// Destination owns any resulting heap buffer; allocator-tag lifetime stays in each caller.
typedef int (*ScriptDiagnosticRangeCall128)(void*, unsigned int, unsigned int, const char*, const char*, void*);

struct Fn8009DB08String {
    unsigned char data[12];
    // Build an owned string from text; mode selects equivalent matched construction paths.
    Fn8009DB08String(const char* text, int mode = 0) {
        if (mode) {
            unsigned int len;
            register Fn8009DB08String* self = this;
            AsMWStringStorage(self)->flags = 0;
            AsMWStringStorage(self)->length = 0;
            *(unsigned int*)&self->data[8] = 0;
            len = strlen(text);
            String_ReserveCapacity((Fn8009BBFCString*)self, len);
            Fn8009DB08Tag copy;
            Fn8009DB08Tag tag;
            copy.value = tag.value;
            ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(self, 0, 0, text, text + len, &copy);
        } else {
            register Fn8009DB08String* self = this;
            AsMWStringStorage(self)->flags = 0;
            AsMWStringStorage(self)->length = 0;
            *(unsigned int*)&self->data[8] = 0;
            register unsigned int len = strlen(text);
            String_ReserveCapacity((Fn8009BBFCString*)self, len);
            Fn8009DB08Tag copy;
            Fn8009DB08Tag tag;
            copy.value = tag.value;
            ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(self, 0, 0, text, text + len, &copy);
        }
    }
    // Release string storage if its tag selects an owned heap buffer.
    ~Fn8009DB08String() {
        if (AsMWStringStorage(this)->flags >> 31)
            appFreeUni__FPv(AsMWStringStorage(this)->data);
    }
    // Copy inline bytes or deep-copy heap text into fresh destination string storage.
    void copyTo(Fn8009BBFCString* out) {
        if ((AsMWStringStorage(this)->flags >> 31) == 0) {
            unsigned int a = AsMWStringStorage(this)->flags;
            unsigned int b = AsMWStringStorage(this)->length;
            AsMWStringStorage(out)->flags = a;
            AsMWStringStorage(out)->length = b;
            *(unsigned int*)&out->data[8] = *(unsigned int*)&data[8];
        } else {
            AsMWStringStorage(out)->flags = 0;
            AsMWStringStorage(out)->length = 0;
            *(unsigned int*)&out->data[8] = 0;
            String_ReserveCapacity(out, AsMWStringStorage(this)->length);
            Fn8009DB08Tag ch;
            Fn8009DB08Tag tag;
            ch.value = tag.value;
            unsigned int size = AsMWStringStorage(this)->length;
            const char* src = AsMWStringStorage(this)->data;
            ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(out, 0, 0, src, src + size, &ch);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
// Copy the selected character suffix used to build a tour voice resource name.
void Character_GetAssetNameSuffix(Fn8009BBFCString* out, signed char index);
#pragma pop

// SC_SND_VS_SE_PLAY: build and play the selected character voice with the _vs suffix.
int EventCommand_PlayTourVoiceVariant5(ScriptCommandContextView* script);

struct Fn8009DEC4String {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Construct an owned narrow string by reserving and copying the supplied text.
    Fn8009DEC4String(const char* text) {
        register Fn8009DEC4String* self = this;
        self->w0 = 0;
        self->len = 0;
        self->heapAddressOrInlineTail = 0;
        register unsigned int n = strlen(text);
        String_ReserveCapacity((Fn8009BBFCString*)self, n);
        Fn8009DB08Tag copy;
        Fn8009DB08Tag tag;
        copy.value = tag.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(self, 0, 0, text, text + n, &copy);
    }
    // Release string storage if its tag selects an owned heap buffer.
    ~Fn8009DEC4String() {
        if (w0 >> 31)
            appFreeUni__FPv((void*)heapAddressOrInlineTail);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Copy one of the decide/win/lose/change/item/versus voice suffixes.
void Event_GetAssetNameSuffix(Fn8009BBFCString* out, int index);
#pragma pop

struct Fn80099A88Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x84];
    void* object;
};

// TA_MOVE_CHARA: publish the character movement mode and target point to the manager.

extern int EventScript_IsCostumePartEquipped(short);
extern int EventScript_IsClubSetEquipped(signed char);
extern signed char EventScript_GetCharacterIndex();

struct Fn8009B654Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[2];
    unsigned char conditionCount;
    unsigned char trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
};

// SC_IS_CHARA: count a true condition when the equipped character matches the operand.

extern signed char EventScript_GetCaddieIndex();

struct Fn8009BTargetArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[2];
    unsigned char conditionCount;
    unsigned char trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[0x80];
    void* object;
};

// SC_IS_CADDIE: count a true condition when the equipped caddie matches the operand.

struct Fn8009BTree {
    char reserved_0[0x34];
    void** node;
};

// SC_IS_BLOCK: compare the requested point block flag with the command boolean.

struct Fn8009BState {
    char reserved_0[4];
    unsigned char requestState;
    char reserved_5[0x0B];
    unsigned int elapsedTicks;
};
struct Fn8009BStateRoot {
    char reserved_0[0x34];
    Fn8009BState* state;
};

// TA_APPEAR_ROAD: enter road-reveal state 3 from state 2 and wait until it changes.

struct Fn80099508Entry {
    char reserved_0[2];
    short value;
};

struct Fn80099508State {
    char reserved_0[0x14];
    unsigned char blockState;
    char reserved_15[3];
    unsigned int blockPoint;
    char reserved_1C[4];
    unsigned int blockTicks;
};

struct Fn80099508Object {
    char reserved_0[0x34];
    Fn80099508State* state;
};

struct Fn80099508Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    Fn80099508Entry* cursor;
    unsigned int count;
    char reserved_420[0x80];
    Fn80099508Object* object;
};

struct Fn80099594Entry {
    unsigned char type;
    short value;
    char reserved_4[4];
};
struct Fn80099594Vector {
    Fn80099594Entry* data;
    unsigned int size;
    unsigned int capacity;
};
struct Fn80099594Object {
    char reserved_0[4];
    Fn80099594Vector entries;
};
struct Fn80099594Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    Fn80099594Entry* cursor;
    char reserved_41C[0x84];
    Fn80099594Object* object;
};

extern void* appMallocDummy__FUl(unsigned int);

struct Fn80099AD8Entry {
    short value;
    unsigned char type;
    char reserved_3[1];
    unsigned int entryKind;
};
struct Fn80099AD8Vector {
    Fn80099AD8Entry* data;
    unsigned int size;
    unsigned int capacity;
};
struct Fn80099AD8Object {
    char reserved_0[0x18];
    Fn80099AD8Vector entries;
};
struct Fn80099AD8Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x84];
    Fn80099AD8Object* object;
};

// TA_BLOCK_POINT: request block-point state 5, update its tick count, and wait for completion.

struct Fn8009B794Arg {
    char reserved_0[2];
    unsigned short flags;
    unsigned char reserved_4[2];
    unsigned char conditionCount;
    unsigned char trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
};

// SC_IS_COSTUME: evaluate the requested costume condition and accumulate its result.

// SC_IS_CLUBSET: evaluate the requested club-set condition and accumulate its result.

struct Fn80094B44Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
};

extern void ConteResource_SetCategoryResourcePair(unsigned char, unsigned char, unsigned int, unsigned int);

// SC_FACE_PARTS: send the character/channel bytes and two word operands to facial-part control.

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

// Destroy owned buffer contents and optionally release the buffer object itself.
int SharedFilePointerVector_Destroy(void* buffer, int deleteStorage);

// Clear the owned-buffer count, free its data, and optionally delete the object.
int ScriptOwnedBuffer_dtor(void* buffer, int deleteStorage);

extern signed char s_sharedFileListInitialized;
extern char s_sharedFileListStorage[];
extern char s_sharedFilePointerVectorDtorRegistration[];
extern void SharedFilePointerVector_InitReserved1024(void*);
extern int SharedFilePointerVector_Destroy(void*, int);
extern void __register_global_object(void*, void (*)(), void*);

#pragma push
#pragma dont_inline on
// Lazily initialize and return the shared file-handle list.
void* ScriptState_GetSharedFileList();
#pragma pop

// Read the node stored in an iterator wrapper.
void* ScriptMapIterator_GetNode(void** iterator);

struct Fn8009A170Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
};

extern signed char s_tourHintWindowInitialized;
extern char s_tourHintWindow[];
extern char s_tourHintWindowDtorRegistration[];
extern void __ct__15CTourHintWindowFv(void*);
extern void CTourHintWindow_dtor();
extern void __register_global_object(void*, void (*)(), void*);

// TA_DELETE_HINT_WIN: clear the tour hint window registered objects.
int ScriptCommand_ClearTourHintObjects(Fn8009A170Arg* script);

struct Fn8009A22CArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int count;
};

extern signed char s_tourHintWindowInitialized;
extern char s_tourHintWindow[];
extern char s_tourHintWindowDtorRegistration[];
extern void __ct__15CTourHintWindowFv(void*);
extern void CTourHintWindow_dtor();
extern void __register_global_object(void*, void (*)(), void*);
extern float kFloat_1_TourWindow;

#pragma opt_propagation off
// Obtain the shared hint window and register its destructor once.
static inline void EnsureHintWindowInstance() {
    if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
}

// TA_SET_WIN_FADE: initialize the main hint fade and wait while updating its opacity.
// The singleton checks implement shared access only. On first tick select fade
// direction and step=duration?255/duration:255 (integer division). Each tick
// broadcasts the PRE-update opacity to navigation/glyphs, then advances the fade
// by 1.0. Return 0 until complete, then advance exactly four bytes. Very large
// durations can produce a zero step; existing behavior is left intact.
int Script_FadeTourHintWindow(Fn8009A22CArg* self);
#pragma opt_propagation reset

struct Fn8009A570Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int count;
};

// TA_SET_SUBWIN_FADE: initialize the hint message fade and wait for completion.
int EventScript_ProcessTourHintFade(Fn8009A570Arg* self);

extern signed char s_bpPlayerInitialized;
extern signed char s_tourHintWindowInitialized;
extern signed char s_tourVsWindowInitialized;
extern char s_bpPlayer[];
extern char s_bpPlayerDtorRegistration_Exhibition[];
extern char s_tourHintWindow[];
extern char s_tourHintWindowDtorRegistration[];
extern char s_tourVSWindow[];
extern char s_tourVsWindowDestructorRegistration[];
extern void __ct__9CBpPlayerFv(void*);
extern void __dt__9CBpPlayerFv();
extern void* StoryMessageScalar_GetValue(void*);
extern void TourMessageOwner_DestroyMessageDraw(void*);
extern void MessageTable_ExpandEntryText(void*, void*, unsigned short, int);
extern void __ct__15CTourHintWindowFv(void*);
extern void CTourHintWindow_dtor();
extern void __ct__13CTourVSWindowFv(void*);
extern void Destroy__13CTourVSWindowFi();
extern void __register_global_object(void*, void (*)(), void*);
extern void TourHintMessage_SetText(void*, void*);
extern void appFreeUni__FPv(void*);
struct Fn8009A790Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int index;
};
struct Fn8009A790Result {
    unsigned int flags;
    unsigned int size;
    unsigned int data;
    // Release the local string buffer if it uses owned heap storage.
    ~Fn8009A790Result() {
        if (flags >> 31)
            appFreeUni__FPv((void*)data);
    }
};
// Expand a hint message id, process its input over ticks, and release its draw object.
int EventScript_ProcessTourHintMessage(Fn8009A790Arg* self);
struct Fn8009B0E0Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int index;
};
extern signed char s_tourVsWindowInitialized;
extern char s_tourVSWindow[];
extern char s_tourVsWindowDestructorRegistration[];
extern float kFloat_1_TourWindow;
struct Fn8009ADBCArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    int count;
};
#pragma opt_propagation off
// Obtain the shared versus window and register its destructor once.
static inline void EnsureVSWindowInstance() {
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
}

// Start the versus-window transition and wait for both its fade and transition to finish.
// First entry begins the transition and a full-step fade-in. Later calls publish
// pre-update group opacity, update the fade, then update the transition ONLY if
// the fade completed. Both must complete before the four-byte command advances.
int Script_StartTourVSWindowTransition(Fn8009ADBCArg* self);
#pragma opt_propagation reset
// Expand a versus message id, process its input over ticks, and release its draw object.
int EventScript_ProcessTourVersusMessage(Fn8009B0E0Arg* self);

struct Fn80099288Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x410];
    unsigned char* base;
    unsigned char* cursor;
};

extern int Int32_CompareByMode(void*, unsigned char, unsigned int);

// SC_VAR_JMP: conditionally jump to a byte-swapped script offset using a bound variable.

struct Fn8009B8A0Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[2];
    unsigned char conditionCount;
    unsigned char trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
    char reserved_41C[0x80];
    void* object;
};

extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern void SaveData_Ctor(void*);
extern void* SaveData_GetCurrentRecord(void*);

// Count a true condition when the saved coin count reaches the unsigned threshold.

#pragma push
#pragma dont_inline on
// Find an exact byte key, returning the embedded end sentinel when it is absent.
void ScriptResourceMap_Find(void** result, void* map, unsigned char* soughtKey);

// Find an exact byte key through a lower-bound walk, or return the end sentinel.
void ScriptByteMap_Find(void** result, void* map, unsigned char* key);

// Count a true condition when the current saved tour has its clear bit set.

struct Fn800993A4Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    unsigned int count;
    char reserved_420[0x24];
    unsigned int pendingModelCount;
    char reserved_448[0x28];
    unsigned int pendingKeyedModelCount;
};

// SC_WAIT_MDL: wait until both the model queue and keyed model map are empty.

struct Fn8009BE3CArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[2];
    unsigned char conditionCount;
    unsigned char trueConditionCount;
    char reserved_8[0x410];
    unsigned char* cursor;
};

extern CSharedDoc* g_sharedDoc;

// Count a true condition when the party result tier matches the signed operand.
int ScriptCondition_CheckPartyResultTier(Fn8009BE3CArg* script);

struct Fn8009911CArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
};

extern void EventFlag_SetStub(unsigned char, unsigned char, unsigned char);

// SC_SET_FLG: forward the three flag operands to the flag setter.

struct Fn80099188Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x3E4];
};

extern void* EventFlag_GetStoragePointerStub(unsigned char, unsigned char);
extern void* EventParameter_GetValueWord(unsigned char, unsigned char);

// SC_SET_VAR_FLG: bind a script variable slot to the selected flag storage.

// SC_SET_VAR_PARA: bind a script variable slot to the selected parameter storage.

struct Fn8009F184Arg {
    char unk0;
    unsigned char index;
    unsigned short flags;
};

extern int ScriptState_ProcessSharedFileCommands(void*, void*);

// Temporarily set the current nesting flag and run shared-file commands until complete.
int ScriptState_RunSharedFileCommandsUntilComplete(Fn8009F184Arg* script, void* applyLoads);

extern signed char s_courseSlotTableInitialized;
extern char s_conteModelSlots[];
extern void CourseSlotTable_InitEmpty(void*);

#pragma push
#pragma dont_inline off
#pragma auto_inline off
// Lazily initialize and return the actor slot table.
void* CourseSlotTable_GetInstance();
#pragma pop

// Copy the iterator node word.
void ScriptMapIterator_Copy(unsigned int* destination, unsigned int* source);

// Construct the end iterator from the map embedded root/sentinel address.
void ScriptMapIterator_SetEnd(void** iterator, char* map);

// Return the byte-key/value pair inside the current map node.
char* MapIter_GetValuePtr(void** iterator);
#pragma pop

extern unsigned int s_scriptByteMapDefaultValue;
extern void* EventFxMap_InsertNode(void*, unsigned int, unsigned int, unsigned int, void*);
extern void* EventFxMap_FindInsertionPosition(void*, unsigned char*, void**, unsigned char*, unsigned char*);

// Return the mapped-value address, inserting the default value for a missing key.
void* ScriptByteMap_GetOrInsertValue(void* map, unsigned char* key);

// Locate a byte-key insertion parent, predecessor, side, and begin-update flag.
void* EventFxMap_FindInsertionPosition(void* map, unsigned char* soughtKey, void** predecessor,
                                       unsigned char* insertLeft, unsigned char* updateBegin);

struct Fn80094EB8Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x70];
    void* object;
};

extern void EventFxMap_EraseByKey(void*, unsigned char);

// SC_FACE_TIME: apply facial timing operands to the interpreter's required +48C face alias.

// SC_EFF_CUT: erase the attached effect with the command byte key.
int EventScript_EraseEffectByKey(Fn80094EB8Arg* script);

struct Fn8009BEA4Arg {
    char reserved_0[2];
    unsigned short flags;
};

// Read interpreter nesting flags, optionally ignoring the outermost condition bit.
unsigned short EventScript_GetConditionFlags(Fn8009BEA4Arg* script, int ignoreOuterCondition);

#pragma push
#pragma dont_inline on
// Return inline or heap character data from a narrow MSL small string.
void* ScriptString_GetCharacterData(void* string);
#pragma pop

// Load an indexed word and reverse its four bytes for script table addressing.
unsigned int WordArray_LoadByteSwapped(unsigned int* words, unsigned char index);

struct Fn8009BDFCArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    char reserved_41C[0x84];
    void* object;
};

// TA_CONTE_SKIP: set the tour-state skip/exit request and advance the script.

struct Fn800994C4Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x84];
    void* object;
};

// TA_SET_START_POINT: store the script start point in the owning manager.

struct Fn80099348Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    unsigned int count;
    char reserved_420[0x18];
    unsigned int pendingCameraCount;
};

// SC_WAIT_CAM: wait until the camera callback queue is empty.

struct Fn8009940CArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    unsigned int count;
    char reserved_420[0x30];
    unsigned int pendingEffectCount;
};

// SC_WAIT_EFF: wait until the effect callback queue is empty.

struct Fn80099468Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    unsigned int count;
    char reserved_420[0x3C];
    unsigned int pendingMessageCount;
};

// SC_WAIT_MSG: wait until the message callback queue is empty.

#pragma push
#pragma dont_inline on
// Compare iterator node words for inequality.
unsigned int ScriptMapIterator_NotEqual(unsigned int* left, unsigned int* right);
#pragma pop

extern unsigned int s_storyConteRequiredFilesStorage[];
extern int FileList_GetLoadedBufferByName(void*, const char*);
extern char __vt__15CEventConteProc[];
extern char* conteCommandNames[];
extern void* __nw__FUl(unsigned int);
extern void __ct__22CEventModelSC_MDL_FACEFPQ23ns015Fn8021F0F4Input(void*, void*);

extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void __dl__FPv(void*);
extern char* strcpy(char*, const char*);

#pragma push
#pragma use_lmw_stmw on
// Return 1 when every required name resolves to a nonnull loaded BUFFER; an empty
// required-name vector also succeeds. This does not inspect file size or validate
// contents. A missing/unloaded name returns 0 at the first null lookup. The lazy
// shared-list initialization below is a side effect even on that failure path.
int StoryConte_AreRequiredFilesAvailable();
#pragma pop

// SC_IS_TOUR_FLAG: test a selected tour-point or record flag and accumulate the condition.
// Selectors 0/1/4 read point bits 12/11/4; 2/3 read record bits 6/7. Quest meanings remain unproven.

// TA_SET_APPEAR_ROAD_INFO: request a road reveal when its variant flag is still unset.

struct Fn80094D74Node {
    Fn80094D74Node* left;
    Fn80094D74Node* right;
    unsigned int parent;
    unsigned char key;
    char padD[3];
    void* payload;
};
struct Fn80094D74Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x74];
    unsigned int flag;
    Fn80094D74Node* root;
};

#pragma opt_common_subs off
// Find the keyed face object and stop the requested facial channel.
inline void Fn80094D74Clear(Fn80094D74Arg* self, unsigned char key, int channel) {
    if (self->flag != 0) {
        Fn80094D74Node* parent = (Fn80094D74Node*)&self->root;
        Fn80094D74Node* node = self->root;
        while (node != 0) {
            unsigned char nkey = node->key;
            if (nkey >= key) {
                parent = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
        if (parent == (Fn80094D74Node*)&self->root || key < parent->key)
            parent = (Fn80094D74Node*)&self->root;
        if (parent != (Fn80094D74Node*)&self->root) {
            void* payload = parent->payload;
            ((CEventFace*)payload)->ApplyLastFrameAndClearTrack(channel);
        }
    }
}
// SC_FACE_STOP: stop both facial animation channels for the selected byte key.
#pragma opt_common_subs reset

// Default iterator construction leaves its storage untouched.
extern void ScriptMapIterator_DefaultConstruct();
extern void* Storage_ClearThreeWords(void*);
extern void ScriptByteMap_Find(void**, void*, unsigned char*);
extern void ScriptMapIterator_Copy(unsigned int*, unsigned int*);
extern void ScriptMapIterator_SetEnd(void**, char*);
extern unsigned int ScriptMapIterator_NotEqual(unsigned int*, unsigned int*);
extern char* MapIter_GetValuePtr(void**);
extern void* ScriptState_GetSharedFileList();
extern void SharedFileList_CreateAndAppend(void*, void*, int);
extern void StringVector_AppendCopy(void*, void*);
extern void EventScript_SweepPendingCallbacks(void*);
extern unsigned int s_storyConteRequiredFilesStorage[];

extern char s_scriptByteKeyMap[];
extern void* scriptOpcodeHandlers[];

struct Fn8009F028Local {
    char data[0x18];
    // Release the local string buffer if it uses owned heap storage.
    ~Fn8009F028Local() {
        ((NString*)(this))->~NString();
    }
};

#pragma push
#pragma use_lmw_stmw on
// Dispatch file-name commands into both shared-file lists; stop when a normal command must wait.
int ScriptState_ProcessSharedFileCommands(void* arg0, void* arg1);
#pragma pop

// EventCommand_InitTourVSWindow
extern signed char s_tourVsWindowInitialized;
extern char s_tourVSWindow[];
extern char s_tourVsWindowDestructorRegistration[];
extern void __ct__13CTourVSWindowFv(void*);
extern void Destroy__13CTourVSWindowFi();
extern void TourVSWindow_SetStatus(void*, int);
extern unsigned int TourVSWindow_HasStatus(void*, int);

struct Fn8009A9C4Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int count;
};

// Request versus-window loading, wait for readiness, then initialize the requested layout.
int EventCommand_InitTourVSWindow(Fn8009A9C4Arg* script);

// EventCommand_FadeOutTourVSWindow
extern signed char s_tourVsWindowInitialized;
extern char s_tourVSWindow[];
extern char s_tourVsWindowDestructorRegistration[];
extern void __ct__13CTourVSWindowFv(void*);
extern void Destroy__13CTourVSWindowFi();
extern void TourVSWindow_StartFadeOut(void*);
extern float kFloat_1_TourWindow;

struct Fn8009AB64Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned int cursor;
    unsigned int count;
};

// Fade out the versus window, wait for completion, then reset and hide it.
int EventCommand_FadeOutTourVSWindow(Fn8009AB64Arg* script);

extern signed char s_tourHintWindowInitialized;
extern char s_tourHintWindow[];
extern char s_tourHintWindowDtorRegistration[];
extern void __ct__15CTourHintWindowFv(void*);
extern void CTourHintWindow_dtor();
extern void __register_global_object(void*, void (*)(), void*);

struct Fn80099FCCArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    int count;
};

// Request hint-window loading, wait for readiness, then select its signed mode operand.
int ScriptCommand_WaitForTourHintState2(Fn80099FCCArg* script);

extern void* ScriptMapIterator_GetNode(void**);
extern void ScriptResourceMap_Find(void**, void*, unsigned char*);
extern void ScriptResourceTree_GetEndIterator(void*, void*);
extern int ScriptTreeIterator_NotEqual(void*, void*);
extern void* __nw__FUl(unsigned int);
extern void* ScriptByteMap_GetOrInsertValue(void*, unsigned char*);
extern void* SlotTable_GetEntry(void*, unsigned char);
extern void* CourseSlotTable_IsExtraSlotRange(void*, unsigned char);
extern unsigned int WordArray_LoadByteSwapped(unsigned int*, unsigned char);
extern char bpPlayerResourceStrings[];
// CEventHandle's constructor, called as a C function so the call site keeps its code shape.
extern int EventScript_RunAndSweep(void*);

// Find or create a face object, resolve the nested script tables, and run its subscript to completion.
// Input is an initialized interpreter with >=8 readable command bytes and a
// live facial-data base at +49C. Command[1] must select a loaded slot 0..4:
// SlotTable_GetEntry (party_main_task_and_phf_helpers.cpp) rejects >=5/null storage, but this reader later
// dereferences the result unchecked. The slot's first byte is an authored index,
// NOT the slot ordinal. Track byte [2] selects the outer offset table as well as
// facial channel (SelectModelAndTrack uses 0 vs nonzero); [3] is loop; [4] is
// subscript. There are no stored row lengths in this reader. Required extents:
//   base: >=(command[2]+1) u32 offsets;
//   chosen outer row: >=2 u32 offsets for range 0/1 (slot<3 / slot>=3);
//   chosen range row: >=(slotRecord[0]+1) u32 offsets;
//   chosen slot row: >=(command[4]+1) u32 offsets.
// Byte selectors have representable domain 0..255; that is NOT proof of 256
// allocated entries. A format mod must supply these extents and valid relative
// offsets within the same retained blob; no global dimensions are established.
// Allocation/child execution failure is not converted to status. The child runs
// without a frame yield until nonzero completion; malformed/cyclic scripts may
// never return. Existing map owns the face; no caller-owned face is transferred.

extern void* EventFxMap_InsertNode(void*, unsigned int, unsigned int, unsigned int, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void RBTree_RebalanceAfterInsert(void*, unsigned int);
extern void appFreeUni__FPv(void*);
extern char* strcpy(char*, const char*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char bpPlayerResourceStrings[];
extern char __dt__Q23std12length_errorFv[];
extern void __throw(void*, void*, void*) __attribute__((noreturn));

extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void __dla__FPv(void*);

struct BufGuard_8009D25C {
    char* data;
    void* alloc;
    // Guard the message array until its shared control block takes ownership.
    BufGuard_8009D25C(char* p, void* a) : data(p), alloc(a) {}
    // Disown and return the guarded message array without freeing it.
    char* release() {
        char* p = data;
        data = 0;
        return p;
    }
    // Free the message array if construction exits before ownership transfer.
    ~BufGuard_8009D25C() {
        if (data)
            __dla__FPv(data);
    }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control MessageControl;
union Alloc_8009D25C {
    volatile unsigned char c;
    unsigned int w;
};
#pragma push
#pragma opt_propagation off
struct ErrBase_8009D25C {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_8009D25C() {
        const char* text = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, treeLengthError);
        Alloc_8009D25C b;
        Alloc_8009D25C a;
        *(void* volatile*)&table = __vt__Q23std11logic_error;
        a.c = 0;
        b.c = 0;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &b);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_8009D25C() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_8009D25C : ErrBase_8009D25C {
    // Construct and throw the byte-tree length error using the runtime type chain.
    length_error_8009D25C() : ErrBase_8009D25C() {
        table = (void*)__vt__Q23std12length_error;
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_8009D25C() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
#pragma pop
}
// Return caller-supplied placement storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}
extern "C" {
// Initialize the tagged parent word while retaining the original exception boundary.
static inline void parent_8009D25C(unsigned int* p, unsigned int value) {
    if (p) {
        try {
            *p = value;
        } catch (...) {
            throw;
        }
    }
}
struct Word_8009D25C {
    unsigned int value;
    // Leave the map value word untouched until explicit initialization.
    Word_8009D25C() {}
    // Store the map value word while preserving the original exception boundary.
    Word_8009D25C(unsigned int v) {
        try {
            value = v;
        } catch (...) {
            throw;
        }
    }
    // Initialize the map value word while retaining its exception boundary.
    void init(register unsigned int v) {
        try {
            value = v;
        } catch (...) {
            throw;
        }
    }
};
struct Value_8009D25C {
    volatile unsigned char key;
    Word_8009D25C value;
    // Copy the byte-keyed map payload and its associated word value.
    Value_8009D25C(const Value_8009D25C& other) {
        register unsigned int v;
        register unsigned int k;
        k = other.key;
        v = other.value.value;
        key = k;
        value.init(v);
    }
};
struct NodeGuard_8009D25C {
    void* alloc;
    void* node;
    // Temporarily own an allocated map node until it is linked into the tree.
    NodeGuard_8009D25C(void* a, void* p) : alloc(a), node(p) {}
    // Free an uncommitted map-node allocation on exceptional exit.
    ~NodeGuard_8009D25C() {
        if (node)
            appFreeUni__FPv(node);
    }
};
#pragma push
#pragma use_lmw_stmw on
// Allocate and link a byte-key node, update map size/begin, and rebalance the tree.
void* EventFxMap_InsertNode(register void* map, register unsigned int parentAddress, register unsigned int insertLeft,
                            register unsigned int updateBegin, register void* keyValue);
#pragma pop

struct Fn80095574Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    char reserved_41C[0x60];
    char text[0];
};
extern signed char s_courseSlotTableInitialized;
extern char s_conteModelSlots[];
extern void CourseSlotTable_InitEmpty(void*);
extern void* SlotTable_GetEntry(void*, unsigned char);
extern int WPuppet_FindNamedBoneEntry(const char*, const char*);
extern int WPuppet_FindOrCacheBone(const char*, const char*);
extern void EventFxMap_SetEffect(void*, unsigned char, void*, void*, void*);
extern void EventFxMap_ReplaceFromNamedBone(void*, unsigned char, void*, const char*, const char*, void*);
extern void EventFxMap_SetBoneAttachedEffect(void*, unsigned char, void*, const char*, const char*, void*);
extern void EventFxMap_ReplaceWithDefaultSource(void*, unsigned char, void*, void*);
extern unsigned int g_sceneManager;
extern void* NamedEffectList_FindByName(unsigned int, const char*);

// Resolve an actor bone, scene object, or default source and attach the named effect at its offset.

struct Fn8009416CArg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int index;
    char reserved_420[4];
    void* pending;
    char reserved_428[0x48];
    unsigned int count;
    void* treeRoot;
    void* treeCache;
};
extern void __ct__22CEventModelSC_MDL_MOVEFPv(void*, void*);
struct Fn8009416CBase {
    virtual void destroy(int) = 0;
    virtual int update(unsigned int) = 0;
};
struct Fn8009416CPending {
    char reserved_0[0x24];
    // Construct the encoded command event in this owned storage.
    Fn8009416CPending(void* source) {
        __ct__22CEventModelSC_MDL_MOVEFPv(this, source);
    }
};
struct Fn8009416CNode {
    Fn8009416CNode* left;
    Fn8009416CNode* right;
    unsigned int parent;
    unsigned char key;
    char reserved_D[3];
    void* value;
};
struct Fn8009416CString {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Fn8009416CString() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return ScriptDiagnosticLength128(this);
    }
};
extern char* conteCommandNames[];
extern char __vt__15CEventConteProc[];
union U4_8009416C {
    unsigned char c;
    unsigned int w;
};
// Owned event wrappers copy the interpreter prefix and opcode name for diagnostics.
struct Fn8009416CItem {
    unsigned int elapsedTicks;
    void* callback;
    Fn8009416CString eventName, commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Fn8009416CItem(Fn8009416CArg* self, unsigned char* cursor) {
        const char* kind;
        const char* prefix;
        unsigned int n1;
        unsigned int n2;
        kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* pending = self->pending;
        prefix = (AsEventScriptStateView(self)->prefixFlags >> 31) == 0 ? ((ScriptPrefixView*)self)->inlinePrefix
                                                                        : AsEventScriptStateView(self)->prefixHeap;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile U4_8009416C a1;
        n1 = eventName.size();
        U4_8009416C c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_8009416C a2;
        n2 = commandName.size();
        U4_8009416C c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
extern void RBTree_RebalanceAfterErase(void*, void*);
extern unsigned int s_scriptByteMapDefaultValue;
extern char* conteCommandNames[];
extern char __vt__15CEventConteProc[];

#pragma opt_common_subs off
// Replace an existing keyed model move, then wait or transfer it to the keyed callback map.
// +8 is the byte key, +0x19 selects synchronous (0) versus asynchronous (nonzero).
// On first tick destroy and erase any old keyed callback before constructing the
// replacement. Sync: interpreter owns pending across 0 returns, then destroys it.
// Async: wrapper/map owns it after insertion and pending becomes null. Both
// completed paths advance 28 bytes; skipped commands advance without side effects.
#pragma opt_common_subs reset

struct Fn80097F94Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pending;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
struct Fn80097F94String {
    unsigned char data[12];
};
extern void __ct__Q23ns112Unk802232OutFPQ23ns117BgmStopScriptData(void*, void*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern char __vt__15CEventConteProc[];
extern char* conteCommandNames[];
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
union U4_80097F94 {
    unsigned char c;
    unsigned int w;
};
struct Node_80097F94 {
    char data[12];
    // Construct the encoded command event in this owned storage.
    Node_80097F94(void* p) {
        __ct__Q23ns112Unk802232OutFPQ23ns117BgmStopScriptData(this, p);
    }
};
struct Str_80097F94 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80097F94() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return ScriptDiagnosticLength128(this);
    }
};
struct Item_80097F94 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80097F94 eventName, commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80097F94(Fn80097F94Arg* self, unsigned char* cursor) {
        unsigned int n2;
        unsigned int n1;
        const char* prefix;
        const char* kind;
        kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* pending = self->pending;
        prefix = (AsEventScriptStateView(self)->prefixFlags >> 31) == 0 ? ((ScriptPrefixView*)self)->inlinePrefix
                                                                        : AsEventScriptStateView(self)->prefixHeap;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile U4_80097F94 a1;
        n1 = eventName.size();
        U4_80097F94 c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_80097F94 a2;
        n2 = commandName.size();
        U4_80097F94 c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_80097F94 {
    unsigned int* first;
    unsigned int count, cap;
    unsigned int* capRef;
    unsigned int prefixOffset;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_80097F94(unsigned int* r) : first(0), count(0), cap(0), capRef(r), prefixOffset(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_80097F94() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_80097F94(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct ErrBase_80097F94 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80097F94(register const char* text) {
        U4_80097F94 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
struct LengthError_80097F94 : ErrBase_80097F94 {
    // Install the length-error vtable after constructing owned diagnostic text.
    LengthError_80097F94(const char* p) : ErrBase_80097F94(p) {
        SetTable(__vt__Q23std12length_error);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80097F94() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80097F94                                                                                              \
    {                                                                                                                  \
        register char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80097F94 ex(msg);                                                                                  \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex,       \
                __dt__Q23std12length_errorFv);                                                                         \
    }

struct Fn8009856CArg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x414];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pending;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
union U4_8009856C {
    unsigned char c;
    unsigned int w;
};
struct Fn8009856CString {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Fn8009856CString() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return ScriptDiagnosticLength128(this);
    }
};
extern void __ct__25CEventSoundSC_SND_BGM_VOLFPQ23ns119BgmVolumeScriptData(void*, void*);
struct Fn8009856CNode {
    char opaqueStorage[0x10];
    // Construct the encoded command event in this owned storage.
    Fn8009856CNode(void* cursor) {
        __ct__25CEventSoundSC_SND_BGM_VOLFPQ23ns119BgmVolumeScriptData(this, cursor);
    }
};
extern char __vt__15CEventConteProc[];
extern char* conteCommandNames[];
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);

struct Item_8009856C {
    unsigned int elapsedTicks;
    void* callback;
    Fn8009856CString eventName, commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_8009856C(Fn8009856CArg* self, unsigned char* cursor) {
        const char* kind;
        const char* prefix;
        unsigned int n1;
        unsigned int n2;
        kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* pending = self->pending;
        prefix = (AsEventScriptStateView(self)->prefixFlags >> 31) == 0 ? ((ScriptPrefixView*)self)->inlinePrefix
                                                                        : AsEventScriptStateView(self)->prefixHeap;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile U4_8009856C a1;
        n1 = eventName.size();
        U4_8009856C c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_8009856C a2;
        n2 = commandName.size();
        U4_8009856C c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct ErrBase_8009856C {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_8009856C(register const char* text) {
        volatile U4_8009856C z;
        z.c = 0;
        table = __vt__Q23std11logic_error;
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, (void*)&z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_8009856C() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
struct LengthError_8009856C : ErrBase_8009856C {
    // Construct and throw the vector length error using the runtime type chain.
    LengthError_8009856C(const char* p) : ErrBase_8009856C(p) {
        table = __vt__Q23std12length_error;
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_8009856C() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_8009856C                                                                                              \
    {                                                                                                                  \
        register char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_8009856C ex(msg);                                                                                  \
    }
// Construct and throw the vector length error through the local ABI expansion.
static inline void length_error_8009856C() {
    LE_THROW_8009856C
}

extern void __ct__Q23ns112Unk802236OutFPQ23ns118BgmPitchScriptData(void*, void*);
// commandTickWord at +0x41C holds command-tick bits; pendingEvent at +0x430
// holds the pending event pointer. Neither field describes a tree here.
struct Fn80098B44Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    char reserved_414[4];
    unsigned char* cursor;
    void* commandTickWord;
    char reserved_420[0x10];
    void* pendingEvent;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
struct Fn80098B44Node {
    char opaqueStorage[0x10];
    // Construct the encoded command event in this owned storage.
    Fn80098B44Node(unsigned char* p) {
        __ct__Q23ns112Unk802236OutFPQ23ns118BgmPitchScriptData(this, p);
    }
};
union U4_80098B44 {
    unsigned char c;
    unsigned int w;
};
struct Fn80098B44String {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Fn80098B44String() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return ScriptDiagnosticLength128(this);
    }
};
extern char* conteCommandNames[];
extern char __vt__15CEventConteProc[];
struct Item_80098B44 {
    unsigned int elapsedTicks;
    void* callback;
    Fn80098B44String eventName, commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80098B44(Fn80098B44Arg* self, unsigned char* cursor) {
        const char* kind;
        const char* prefix;
        unsigned int n1;
        unsigned int n2;
        kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* pending = self->pendingEvent;
        prefix = (self->mode >> 31) == 0 ? ((ScriptPrefixView*)self)->inlinePrefix : self->prefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile U4_80098B44 a1;
        n1 = eventName.size();
        U4_80098B44 c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_80098B44 a2;
        n2 = commandName.size();
        U4_80098B44 c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct ErrBase_80098B44 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80098B44(register const char* text) {
        volatile U4_80098B44 z;
        z.c = 0;
        table = __vt__Q23std11logic_error;
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, (void*)&z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80098B44() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
struct LengthError_80098B44 : ErrBase_80098B44 {
    // Construct and throw the vector length error using the runtime type chain.
    LengthError_80098B44(const char* p) : ErrBase_80098B44(p) {
        table = __vt__Q23std12length_error;
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80098B44() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80098B44                                                                                              \
    {                                                                                                                  \
        register char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80098B44 ex(msg);                                                                                  \
    }
// Construct and throw the vector length error through the local ABI expansion.
static inline void length_error_80098B44() {
    LE_THROW_80098B44
}

struct Fn800967A4Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pendingSound;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
extern void __ct__Q23ns016EventSoundSePlayFPCc(void*, void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
struct Fn800967A4Node {
    char opaqueStorage[0x18];
    // Construct the encoded command event in this owned storage.
    Fn800967A4Node(void* p) {
        __ct__Q23ns016EventSoundSePlayFPCc(this, p);
    }
};
union U4_800967A4 {
    unsigned char c;
    unsigned int w;
};
struct Str_800967A4 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_800967A4() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_800967A4 {
    unsigned int elapsedTicks;
    void* callback;
    Str_800967A4 eventName;
    Str_800967A4 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_800967A4(Fn800967A4Arg* self, unsigned char* cursor) {
        void* n0 = self->pendingSound;
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        char* prefix;
        if ((self->mode >> 31) == 0)
            prefix = ((ScriptPrefixView*)self)->inlinePrefix;
        else
            prefix = self->prefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = n0;
        volatile U4_800967A4 a1;
        unsigned int n1 = eventName.size();
        U4_800967A4 c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_800967A4 a2;
        unsigned int n2 = commandName.size();
        U4_800967A4 c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_800967A4 {
    unsigned int* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_800967A4(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_800967A4() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_800967A4(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct ErrBase_800967A4 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_800967A4(const char* text) {
        U4_800967A4 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
namespace std {
struct length_error_800967A4 : ErrBase_800967A4 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_800967A4(const char* p) : ErrBase_800967A4(p) {
        SetTable(__vt__Q23std12length_error);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_800967A4() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_800967A4                                                                                        \
    {                                                                                                            \
        char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError);    \
        std::length_error_800967A4 ex(msg);                                                                      \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }
#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect play event and transfer it to the sound callback queue.
#pragma pop
extern void __ct__Q23ns015EventSoundSeReqFPCc(void*, void*);
union U4_800961A8 {
    unsigned char c;
    unsigned int w;
};
struct Fn800961A8Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pendingSound;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
struct Fn800961A8Node {
    char opaqueStorage[0x14];
    // Construct the encoded command event in this owned storage.
    Fn800961A8Node(void* p) {
        __ct__Q23ns015EventSoundSeReqFPCc(this, p);
    }
};
struct Str_800961A8 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_800961A8() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_800961A8 {
    unsigned int elapsedTicks;
    void* callback;
    Str_800961A8 eventName;
    Str_800961A8 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_800961A8(Fn800961A8Arg* self, unsigned char* cursor) {
        void* n0 = self->pendingSound;
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        char* prefix = ((ScriptPrefixView*)self)->inlinePrefix;
        if (self->mode >> 31)
            prefix = self->prefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = n0;
        volatile U4_800961A8 a1;
        unsigned int n1 = eventName.size();
        U4_800961A8 c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_800961A8 a2;
        unsigned int n2 = commandName.size();
        U4_800961A8 c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_800961A8 {
    unsigned int* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_800961A8(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_800961A8() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_800961A8(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct ErrBase_800961A8 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_800961A8(const char* text) {
        U4_800961A8 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
namespace std {
struct length_error_800961A8 : ErrBase_800961A8 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_800961A8(const char* p) : ErrBase_800961A8(p) {
        SetTable(__vt__Q23std12length_error);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_800961A8() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_800961A8                                                                                     \
    {                                                                                                         \
        char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        std::length_error_800961A8 ex(msg);                                                                   \
        __throw(msg + 0x14, &ex, __dt__Q23std12length_errorFv);                                               \
    }
#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect request event and transfer it to the sound callback queue.
#pragma pop

extern void __ct__26CEventSoundSC_SND_BGM_PLAYFPQ23ns117BgmPlayScriptData(void*, void*);
extern char* conteCommandNames[];
extern char __vt__15CEventConteProc[];
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
struct Fn80097998Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pending;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
struct Fn80097998Node {
    char opaqueStorage[0x18];
    // Construct the encoded command event in this owned storage.
    Fn80097998Node(void* p) {
        __ct__26CEventSoundSC_SND_BGM_PLAYFPQ23ns117BgmPlayScriptData(this, p);
    }
};
struct Str_80097998 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80097998() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_80097998 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80097998 eventName;
    Str_80097998 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80097998(Fn80097998Arg* self, unsigned char* cursor) {
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->mode >> 31) ? self->prefix : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = self->pending;
        volatile unsigned char a1;
        unsigned int n1 = eventName.size();
        unsigned char c1 = a1;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile unsigned char a2;
        unsigned int n2 = commandName.size();
        unsigned char c2 = a2;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_80097998 {
    unsigned int* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_80097998(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_80097998() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_80097998(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct ErrBase_80097998 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80097998(const char* text) {
        unsigned char z = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80097998() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_80097998 : ErrBase_80097998 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_80097998(const char* p) : ErrBase_80097998(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_80097998() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80097998                                                                                        \
    {                                                                                                            \
        std::length_error_80097998 ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }
// Create a named BGM play event and transfer it to the sound callback queue.

extern void __ct__Q23ns016EventSoundSeStopFPCc(void*, void*);
extern void __ct__Q23ns017EventSoundSePitchFPCc(void*, void*);
struct Fn8009739CArg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pending;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
struct Fn8009739CNode {
    char opaqueStorage[0x1c];
    // Construct the encoded command event in this owned storage.
    Fn8009739CNode(void* p) {
        __ct__Q23ns017EventSoundSePitchFPCc(this, p);
    }
};
extern char* conteCommandNames[];
extern char __vt__15CEventConteProc[];
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
union U4_8009739C {
    unsigned char c;
    unsigned int w;
};
struct Item_8009739C {
    unsigned int elapsedTicks;
    void* callback;
    Str_800967A4 eventName;
    Str_800967A4 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_8009739C(Fn8009739CArg* self, unsigned char* cursor) {
        register const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* pending = self->pending;
        register const char* prefix = ((ScriptPrefixView*)self)->inlinePrefix;
        if (self->mode >> 31)
            prefix = self->prefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile U4_8009739C a1;
        unsigned int n1 = eventName.size();
        U4_8009739C c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile U4_8009739C a2;
        unsigned int n2 = commandName.size();
        U4_8009739C c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct ErrBase_8009739C {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_8009739C(register const char* text) {
        U4_8009739C z;
        z.c = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_8009739C() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_8009739C : ErrBase_8009739C {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_8009739C(const char* p) : ErrBase_8009739C(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_8009739C() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_8009739C                                                                                        \
    {                                                                                                            \
        std::length_error_8009739C ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }

struct Fn80096DA0Arg {
    unsigned char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int mode;
    unsigned char reserved_40C[4];
    char* prefix;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int commandTick;
    char reserved_420[0x10];
    void* pending;
    char reserved_434[0x30];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
#pragma push
#pragma use_lmw_stmw on
struct Fn80096DA0Node {
    char opaqueStorage[0x14];
    // Construct the encoded command event in this owned storage.
    Fn80096DA0Node(void* p) {
        __ct__Q23ns016EventSoundSeStopFPCc(this, p);
    }
};
struct Al4_80096DA0 {
    unsigned char v;
    char pad[3];
};
struct ErrBase_80096DA0 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80096DA0(const char* text) {
        Al4_80096DA0 z;
        z.v = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z.v);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80096DA0() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_80096DA0 : ErrBase_80096DA0 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_80096DA0(const char* p) : ErrBase_80096DA0(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_80096DA0() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80096DA0                                                                                        \
    {                                                                                                            \
        std::length_error_80096DA0 ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }
struct Item_80096DA0 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80097998 eventName;
    Str_80097998 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80096DA0(Fn80097998Arg* self, unsigned char* cursor) {
        void* pendingNode = self->pending;
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->mode >> 31) ? self->prefix : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pendingNode;
        volatile Al4_80096DA0 a1;
        volatile unsigned int* pw0 = &eventName.w0;
        unsigned int n1 = (*pw0 >> 31) == 0 ? (*(unsigned char*)pw0 & 0x7F) : eventName.len;
        Al4_80096DA0 c1;
        c1.v = a1.v;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile Al4_80096DA0 a2;
        unsigned int n2 = commandName.size();
        Al4_80096DA0 c2;
        c2.v = a2.v;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
// Create a named sound-effect stop event and transfer it to the sound callback queue.
#pragma pop

extern char s_storyWindowStaticStorage[];
extern char s_sharedFileListStorage[];
extern void SharedFileHandleList_RemoveByDataName(void*, const char*);
extern signed char s_sharedFileListInitialized;
extern int SharedFilePointerVector_Destroy(void*, int);
extern void __register_global_object(void*, void (*)(), void*);

struct Fn8009F2E4Item {
    unsigned int flag;
    unsigned int value;
    void* ptr;
};

// Clear file handlers, remove required files from the shared list, and free the name vector.
void ScriptSharedFiles_RemoveRequiredFilesAndRelease();

struct Fn80092804Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int flag;
    char reserved_40C[4];
    void* strPtr;
    char reserved_414[4];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[4];
    void* object;
    char reserved_428[0x18];
    void** items;
    unsigned int size;
    unsigned int capacity;
};

struct Fn80095B54Arg {
    char reserved_0[2];
    unsigned short flags;
    char reserved_4[0x404];
    unsigned int flag;
    char reserved_40C[4];
    char* strPtr;
    char reserved_414[4];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[0x0C];
    void* object;
    char reserved_430[0x28];
    void** items;
    unsigned int size;
    unsigned int capacity;
};

extern void __ct__22CEventModelSC_MDL_HEADFPQ23ns015Fn8021F4A4Input(void*, void*);
extern void __ct__22CEventModelSC_MDL_FACEFPQ23ns015Fn8021F0F4Input(void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern char __vt__15CEventConteProc[];
extern char* conteCommandNames[];
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
struct Fn80092E58Node {
    char pad[0x2C];
    // Construct the encoded command event in this owned storage.
    Fn80092E58Node(void* p) {
        __ct__22CEventModelSC_MDL_HEADFPQ23ns015Fn8021F4A4Input(this, p);
    }
};
struct Fn80092804Node {
    char pad[0x24];
    // Construct the encoded command event in this owned storage.
    Fn80092804Node(void* p) {
        __ct__22CEventModelSC_MDL_FACEFPQ23ns015Fn8021F0F4Input(this, p);
    }
};

struct Alloc_80092E58 {
    unsigned char value;
    char pad[3];
};
struct ErrBase_80092E58 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80092E58(const char* text) {
        Alloc_80092E58 alloc;
        alloc.value = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &alloc);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80092E58() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
struct LengthError_80092E58 : ErrBase_80092E58 {
    // Construct and throw the vector length error using the runtime type chain.
    LengthError_80092E58(const char* text) : ErrBase_80092E58(text) {
        table = (void*)__vt__Q23std12length_error;
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80092E58() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80092E58                                                                                         \
    {                                                                                                             \
        char* message = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80092E58 ex(message);                                                                         \
    }
// Construct and throw the vector length error through the local ABI expansion.
static inline void length_error_80092E58() {
    LE_THROW_80092E58
}
struct Interface_80092E58 {
    virtual ~Interface_80092E58();
    virtual int check(unsigned int);
};

struct Str_80092E58 {
    unsigned int first, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80092E58() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        volatile unsigned int* p = &first;
        return (*p >> 31) == 0 ? (*(unsigned char*)p & 0x7f) : len;
    }
};
struct Item_80092E58 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80092E58 eventName;
    Str_80092E58 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80092E58(Fn80092804Arg* owner, unsigned char* cur) {
        char* kind;
        char* prefix;
        kind = conteCommandNames[cur[0]];
        void* n = owner->object;
        prefix = (owner->flag >> 31) == 0 ? ((ScriptPrefixView*)owner)->inlinePrefix : (char*)owner->strPtr;
        vtable = __vt__15CEventConteProc;
        eventName.first = 0;
        eventName.len = 0;
        eventName.heapAddressOrInlineTail = 0;
        commandName.first = 0;
        commandName.len = 0;
        commandName.heapAddressOrInlineTail = 0;
        elapsedTicks = 0;
        callback = n;
        volatile Alloc_80092E58 a0;
        unsigned int n0 = eventName.size();
        Alloc_80092E58 c0;
        c0.value = a0.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n0, prefix, prefix + strlen(prefix), &c0);
        volatile Alloc_80092E58 a1;
        unsigned int n1 = commandName.size();
        Alloc_80092E58 c1;
        c1.value = a1.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n1, kind, kind + strlen(kind), &c1);
    }
};
struct Vec_80092E58 {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* capref;
    unsigned int oldSize;
    // Initialize an empty guarded allocation for vector growth.
    Vec_80092E58(unsigned int* p) : data(0), size(0), capacity(0), capref(p), oldSize(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~Vec_80092E58() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Run the model-head event until complete, or transfer it to the model callback queue.

namespace std {
struct length_error : ErrBase_80097998 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error(const char* p) : ErrBase_80097998(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
    DECOMP_CT_MEMBER(length_error)
};
}
struct Str_80092804 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80092804() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_80092804 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80092804 eventName;
    Str_80092804 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80092804(Fn80092804Arg* self, unsigned char* cursor) {
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->flag >> 31) ? (char*)self->strPtr : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = self->object;
        volatile unsigned char a1;
        unsigned int n1 = eventName.size();
        unsigned char c1 = a1;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile unsigned char a2;
        unsigned int n2 = commandName.size();
        unsigned char c2 = a2;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Run the facial-expression event until complete, or transfer it to the model callback queue.
#pragma pop

extern void __ct__23CEventMessageSC_MSG_WINFPUc(void*, unsigned char*);
struct Fn80095B54Node {
    void* vtable;
    void* cursor;
    // Construct the encoded command event in this owned storage.
    Fn80095B54Node(unsigned char* p) {
        __ct__23CEventMessageSC_MSG_WINFPUc(this, p);
    }
};
struct Fn80095B54Base {
    virtual void destroy(int);
    virtual int check(unsigned int);
};

union Char_80095B54 {
    unsigned char c;
    unsigned int align;
};
struct ErrBase_80095B54 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80095B54(register const char* text) {
        volatile Char_80095B54 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, (void*)&z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
struct LengthError_80095B54 : ErrBase_80095B54 {
    // Install the length-error vtable after constructing owned diagnostic text.
    LengthError_80095B54(register const char* p) : ErrBase_80095B54(p) {
        SetTable(__vt__Q23std12length_error);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80095B54() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80095B54                                                                                     \
    {                                                                                                         \
        char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80095B54 ex(msg);                                                                         \
        __throw(msg + 0x14, &ex, __dt__Q23std12length_errorFv);                                               \
    }
struct Str_80095B54 {
    unsigned int w0;
    unsigned int len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80095B54() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : ((volatile Str_80095B54*)this)->len;
    }
};
struct Item_80095B54 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80095B54 eventName;
    Str_80095B54 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80095B54(Fn80095B54Arg* self, unsigned char* cursor) {
        unsigned int n2;
        unsigned int n1;
        const char* prefix;
        const char* kind;
        void* pending = self->object;
        kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        prefix = (self->flag >> 31) == 0 ? ((ScriptPrefixView*)self)->inlinePrefix : self->strPtr;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = pending;
        volatile Char_80095B54 a1;
        n1 = eventName.size();
        Char_80095B54 c1;
        c1.c = a1.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile Char_80095B54 a2;
        n2 = commandName.size();
        Char_80095B54 c2;
        c2.c = a2.c;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};

struct Fn800934ACArg {
    char reserved_0[2];
    unsigned short value;
    char reserved_4[0x404];
    unsigned int flag;
    unsigned char reserved_40C[4];
    char* strPtr;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[4];
    void* object;
    char reserved_428[0x18];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
extern void __ct__28CEventModelSC_MDL_HEAD_RESETFPQ23ns015Fn8021FAB4Input(void*, void*);
struct Fn800934ACNode {
    char pad[0x2C];
    // Construct the encoded command event in this owned storage.
    Fn800934ACNode(void* p) {
        __ct__28CEventModelSC_MDL_HEAD_RESETFPQ23ns015Fn8021FAB4Input(this, p);
    }
};
struct Fn800934ACItem {
    unsigned int elapsedTicks;
    void* node;
    Fn8009BBFCString first;
    Fn8009BBFCString second;
    void* vtable;
};
extern char __vt__15CEventConteProc[];
extern char* conteCommandNames[];

struct Str_800934AC {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_800934AC() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_800934AC {
    unsigned int elapsedTicks;
    void* callback;
    Str_800934AC eventName;
    Str_800934AC commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_800934AC(Fn800934ACArg* self, unsigned char* cursor) {
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->flag >> 31) ? self->strPtr : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = self->object;
        volatile unsigned char a1;
        unsigned int n1 = eventName.size();
        unsigned char c1 = a1;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile unsigned char a2;
        unsigned int n2 = commandName.size();
        unsigned char c2 = a2;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_800934AC {
    unsigned int* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_800934AC(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_800934AC() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_800934AC(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

struct ErrBase_800934AC {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_800934AC(const char* text) {
        unsigned char z = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_800934AC() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_800934AC : ErrBase_800934AC {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_800934AC(const char* p) : ErrBase_800934AC(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_800934AC() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}

// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_800934AC                                                                                        \
    {                                                                                                            \
        std::length_error_800934AC ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }

// Run the head-reset event until complete, or transfer it to the model callback queue.

struct Fn80094F20Arg {
    char reserved_0[2];
    unsigned short value;
    char reserved_4[0x404];
    unsigned int flag;
    unsigned char reserved_40C[4];
    char* strPtr;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[8];
    void* object;
    char reserved_42C[0x20];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
extern void __ct__23CEventEffectSC_EFF_FADEFPv(void*, void*);
struct Fn80094F20Node {
    char pad[12];
    // Construct the encoded command event in this owned storage.
    Fn80094F20Node(void* p) {
        __ct__23CEventEffectSC_EFF_FADEFPv(this, p);
    }
};

struct Str_80094F20 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80094F20() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_80094F20 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80094F20 eventName;
    Str_80094F20 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80094F20(Fn80094F20Arg* self, unsigned char* cursor) {
        void* n0 = self->object;
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->flag >> 31) ? self->strPtr : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = n0;
        volatile unsigned char a1;
        unsigned int n1 = eventName.size();
        unsigned char c1 = a1;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile unsigned char a2;
        unsigned int n2 = commandName.size();
        unsigned char c2 = a2;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};
struct VecBuf_80094F20 {
    unsigned int* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_80094F20(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Free guarded backing storage without destroying transferred callback handles.
    ~VecBuf_80094F20() {
        if (first) {
            count = 0;
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_80094F20(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

struct ErrBase_80094F20 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80094F20(const char* text) {
        unsigned char z = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80094F20() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_80094F20 : ErrBase_80094F20 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_80094F20(const char* p) : ErrBase_80094F20(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_80094F20() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}

// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80094F20                                                                                        \
    {                                                                                                            \
        std::length_error_80094F20 ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }

#pragma push
#pragma use_lmw_stmw on
// Run the effect-fade event until complete, or transfer it to the effect callback queue.
#pragma pop

struct Fn800944F0Arg {
    char reserved_0[2];
    unsigned short value;
    char reserved_4[0x404];
    unsigned int flag;
    unsigned char reserved_40C[4];
    char* strPtr;
    unsigned char reserved_414[4];
    unsigned char* cursor;
    unsigned int count;
    char reserved_420[4];
    void* object;
    char reserved_428[0x18];
    void** items;
    unsigned int size;
    unsigned int capacity;
};
extern void __ct__21CEventModelSC_MDL_ROTFPQ23ns18Input209(void*, void*);
struct Fn800944F0Node {
    char pad[0x24];
    // Construct the encoded command event in this owned storage.
    Fn800944F0Node(void* p) {
        __ct__21CEventModelSC_MDL_ROTFPQ23ns18Input209(this, p);
    }
};

struct Item_800944F0 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80094F20 eventName;
    Str_80094F20 commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_800944F0(Fn800944F0Arg* self, unsigned char* cursor) {
        void* n0 = self->object;
        const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        const char* prefix = (self->flag >> 31) ? self->strPtr : ((ScriptPrefixView*)self)->inlinePrefix;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = n0;
        volatile unsigned char a1;
        unsigned int n1 = eventName.size();
        unsigned char c1 = a1;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&eventName, 0, n1, prefix, prefix + strlen(prefix), &c1);
        volatile unsigned char a2;
        unsigned int n2 = commandName.size();
        unsigned char c2 = a2;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&commandName, 0, n2, kind, kind + strlen(kind), &c2);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Run the model-rotation event until complete, or transfer it to the model callback queue.
#pragma pop
}
extern "C" {
union U4_80099594 {
    volatile unsigned char c;
    unsigned int w;
};
struct Entry_80099594 {
    short value;
    short pad;
    unsigned int type;
    // Construct the short value/type-ID record without interpreting the payload.
    Entry_80099594(short v, unsigned int t) : value(v), type(t) {}
    // Store the short value and type ID in this command record.
    void init(short v, unsigned int t) {
        value = v;
        type = t;
    }
    // Copy all eight record bytes, including padding, with the original EH boundary.
    Entry_80099594(const Entry_80099594& other) {
        try {
            *(unsigned long long*)this = *(const unsigned long long*)&other;
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
};
struct Vec_80099594 {
    Entry_80099594* first;
    unsigned int size, cap;
};
struct Buf_80099594 {
    Entry_80099594* first;
    unsigned int count, cap;
    unsigned int* capRef;
    unsigned int used;
    // Start an empty guarded allocation for eight-byte command records.
    Buf_80099594(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Clear the temporary record count and free its allocation; records own no payload.
    ~Buf_80099594() {
        U4_80099594 z;
        z.c = 0;
        if (first) {
            U4_80099594 z2;
            z2.c = 0;
            unsigned int n = count;
            count -= n;
            appFreeUni__FPv(first);
        }
    }
};
struct ErrBase_80099594 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80099594(const char* text) {
        U4_80099594 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
struct LengthError_80099594 : ErrBase_80099594 {
    // Construct and throw the vector length error using the runtime type chain.
    LengthError_80099594(const char* text) : ErrBase_80099594(text) {
        SetTable(__vt__Q23std12length_error);
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80099594() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80099594                                                                                              \
    {                                                                                                                  \
        U4_80099594 z;                                                                                                 \
        z.c = 0;                                                                                                       \
        register char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80099594 ex(msg);                                                                                  \
    }

struct Object_80099AD8 {
    char pad[0x18];
    Entry_80099594* first;
    unsigned int size, cap;
};
struct ErrBase_80099AD8 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80099AD8(const char* text) {
        U4_80099594 z;
        z.c = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned int len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
struct LengthError_80099AD8 : ErrBase_80099AD8 {
    // Construct and throw the vector length error using the runtime type chain.
    LengthError_80099AD8(const char* text) : ErrBase_80099AD8(text) {
        SetTable(__vt__Q23std12length_error);
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), this,
                __dt__Q23std12length_errorFv);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~LengthError_80099AD8() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80099AD8                                                                                              \
    {                                                                                                                  \
        U4_80099594 z;                                                                                                 \
        z.c = 0;                                                                                                       \
        register char* msg = bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError); \
        LengthError_80099AD8 ex(msg);                                                                                  \
    }

struct Al4_8009D428 {
    unsigned char v;
    char pad[3];
};
struct Str_8009D428 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Copy inline string bytes or deep-copy the source heap text under the original EH boundary.
    Str_8009D428(const Str_8009D428& o) {
        try {
            if ((o.w0 >> 31) == 0) {
                w0 = o.w0;
                unsigned int b = o.len, c = o.heapAddressOrInlineTail;
                len = b;
                heapAddressOrInlineTail = c;
            } else {
                unsigned int sz = o.len;
                w0 = 0;
                len = 0;
                heapAddressOrInlineTail = 0;
                String_ReserveCapacity((Fn8009BBFCString*)this, sz);
                volatile Al4_8009D428 a;
                Al4_8009D428 c;
                c.v = a.v;
                ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(this, 0, 0, (const char*)o.heapAddressOrInlineTail,
                                                                    (const char*)o.heapAddressOrInlineTail + o.len, &c);
            }
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_8009D428() {
        if (w0 >> 31)
            appFreeUni__FPv((void*)heapAddressOrInlineTail);
    }
};
struct Vec_8009D428 {
    Str_8009D428* first;
    unsigned int size;
    unsigned int cap;
};
struct VecBuf_8009D428 {
    Str_8009D428* first;
    unsigned int count;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int used;
    // Initialize an empty guarded allocation for vector growth.
    VecBuf_8009D428(unsigned int* r) : first(0), count(0), cap(0), capRef(r), used(0) {}
    // Destroy constructed elements and free the guarded vector allocation.
    ~VecBuf_8009D428() {
        if (first) {
            unsigned int n = count;
            Str_8009D428* p = first + n;
            count -= n;
            while (n--) {
                --p;
                if (p)
                    p->~Str_8009D428();
            }
            appFreeUni__FPv(first);
        }
    }
};
// Return the larger borrowed growth candidate using the original tie order.
static inline const unsigned int& max_8009D428(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct ErrBase_8009D428 : std::exception {
    // Install the exception vtable required by the runtime ABI.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_8009D428(const char* text) {
        Al4_8009D428 z;
        z.v = 0;
        SetTable(__vt__Q23std11logic_error);
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z.v);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
};
namespace std {
struct length_error_8009D428 : ErrBase_8009D428 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_8009D428(const char* p) : ErrBase_8009D428(p) {
        SetTable(__vt__Q23std12length_error);
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_8009D428() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_8009D428                                                                                        \
    {                                                                                                            \
        std::length_error_8009D428 ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }

#pragma push
#pragma use_lmw_stmw on
// Append a deep copy of a narrow string, growing the vector with exception-safe ownership.
// vector owns the resulting copy; the input string stays borrowed and may alias
// an existing element. Constructing the new tail BEFORE copying the old range
// preserves that source during growth. size/capacity count 12-byte strings.
void StringVector_AppendCopy(void* vector, void* string);
#pragma pop

#pragma push
#pragma use_lmw_stmw on
extern char __vt__12CEventHandle[];
struct ScriptPrefixStorage {
    unsigned int flags, size, heapAddressOrInlineTail;
};
struct ScriptCallbackQueueStorage {
    void* data;
    unsigned int size, capacity;
};
struct ScriptTreeStorage {
    unsigned int size;
    ScriptTreeLinks* root;
    ScriptTreeLinks** begin;
};
struct ScriptInterpreterStorage {
    char pad0[0x408];
    ScriptPrefixStorage first;
    void* user0;
    void* user1;
    unsigned int state[6];
    ScriptCallbackQueueStorage vectors[5];
    ScriptTreeStorage tree0;
    char sub[0x10];
    unsigned int count;
    ScriptTreeStorage tree1;
    void* tail[2];
    void* vtable;
};
// Constructor view -> other views in this file / EventScriptStateView.h:
// +000..007 pad0       flags/condition counters (cleared by first memset)
// +008..407 pad0       256 borrowed object slots (cleared by second memset)
// +408 first          prefix/name: 12-byte tagged MSL string, owns copied text
// +414 user0          borrowed scriptData constructor argument
// +418 user1          current command cursor, initialized from entryAddress
// +41C state[0]       elapsedCommandTicks in shared view; per-command elapsed polling ticks
// +420 state[1]       owned command-handler slot0; destructor contract below
// +424 state[2]       pending model event (Fn8009416CArg/Fn80093B00State)
// +428 state[3]       pending effect event (Fn80094F20Arg.object: effect fade)
// +42C state[4]       pending message request (Fn80095808Arg.root)
// +430 state[5]       pending sound event (e.g. Fn80097F94Arg.pending)
// +434 vectors[0]     ScriptQueueView.cameraEvents
// +440 vectors[1]     ScriptQueueView.modelEvents
// +44C vectors[2]     ScriptQueueView.effectEvents
// +458 vectors[3]     ScriptQueueView.messageEvents
// +464 vectors[4]     ScriptQueueView.soundEvents
// +470 tree0          keyedModelEvents / ScriptObjectMapView.objects
// +47C sub            CEventEffect subobject / ScriptEffectMapView.effectMap
// +48C count          pendingObject in shared view; face alias during subscript
// +490 tree1          ScriptResourceMapView.resources (retained face objects)
// +49C tail[0]        faceScriptTableBase in shared view; initialized to NULL here
// +4A0 tail[1]        manager in shared view; initialized to NULL here
// +4A4 vtable         eventHandleVtable; complete allocation is 0x4A8 bytes
// The command-specific pending fields and +48C face alias must not be conflated.
// +414 borrows the active event script; +49C borrows the separate facial-data
// script. Producer: CStoryConte_OnProcess (story_conte_cutscene.cpp), transition to kStoryPlaying,
// stores eventDataLoad->buffer at handle+49C AFTER constructing the handle and
// before playback; ExecuteFaceSubscript uses its four relative-offset tables.
// That loader retains the data; the interpreter never frees these two buffers.
// Nested face interpreters receive this facial-data base at +414 but leave
// +49C null: recursively nesting another face-subscript requires another writer,
// which is not supplied by this constructor. Do not assume it inherits +49C.
// +420 has no opcode producer in this unit. It is nevertheless LIVE owned state:
// CEventHandle's destructor (story_conte_cutscene.cpp) invokes its virtual deleting destructor then
// clears it, just as for +424/+428/+42C/+430. It is not padding or a counter;
// assigning an event here transfers responsibility for eventual destruction.
// Each vector owns its pointer buffer and published wrappers. Map begin points
// to the root slot when empty, NOT null; replacing it with null breaks iteration.
enum ScriptQueueIndex {
    kCameraQueueIndex = 0,
    kModelQueueIndex = 1,
    kEffectQueueIndex = 2,
    kMessageQueueIndex = 3,
    kSoundQueueIndex = 4
};
// Indices retain the six-word constructor representation without hiding the
// distinct lifetime of the clock and five owned pending command-handler slots.
enum ScriptStateWordIndex {
    kCommandTicksWord = 0,
    kOwnedHandler0Word = 1,
    kPendingModelWord = 2,
    kPendingEffectWord = 3,
    kPendingMessageWord = 4,
    kPendingSoundWord = 5
};
typedef char Interpreter_prefix_offset[((int)&((ScriptInterpreterStorage*)0)->first == 0x408) ? 1 : -1];
typedef char Interpreter_script_argument_offset[((int)&((ScriptInterpreterStorage*)0)->user0 == 0x414) ? 1 : -1];
typedef char Interpreter_cursor_offset[((int)&((ScriptInterpreterStorage*)0)->user1 == 0x418) ? 1 : -1];
typedef char Interpreter_queue_offset[((int)&((ScriptInterpreterStorage*)0)->vectors == 0x434) ? 1 : -1];
typedef char Interpreter_keyed_map_offset[((int)&((ScriptInterpreterStorage*)0)->tree0 == 0x470) ? 1 : -1];
typedef char Interpreter_face_map_offset[((int)&((ScriptInterpreterStorage*)0)->tree1 == 0x490) ? 1 : -1];
typedef char Interpreter_size[(sizeof(ScriptInterpreterStorage) == 0x4A8) ? 1 : -1];
#pragma pop

// Runtime queue and keyed-map schemas are defined once below. Older unused
// address-derived Object/Node declarations described a nonexistent sixth queue
// and a parent-first tree layout; neither describes the five live queues here.
extern int fn_8009E1B4Check(void*, unsigned int);
extern void EventFxMap_UpdateAttachedPositions(void*);

#pragma push
#pragma use_lmw_stmw on
struct E1B4Obj;
struct E1B4VT {
    void* pad0;
    void* pad1;
    void (*dtor)(void*, int);
    int (*check)(E1B4Obj*, unsigned int);
};
struct E1B4Obj {
    E1B4VT* vt;
};
struct E1B4Sub {
    char pad[0x20];
    E1B4VT* vt;
};
// elapsedTicks is saturated polling time in ticks; it is not the map's key.
struct E1B4Item {
    unsigned int elapsedTicks;
    E1B4Obj* obj;
};
struct E1B4Vec {
    E1B4Item** data;
    unsigned int size;
    unsigned int cap;
};
struct E1B4Node {
    E1B4Node* left;
    E1B4Node* right;
    unsigned int parent;
    unsigned int pad;
    E1B4Item* item;
};
struct E1B4Tree {
    unsigned int size;
    E1B4Node* root;
    E1B4Node* begin;
};
// Tree views are compatible PREFIXES, not alternative full object layouts:
// ScriptTreeLinks: +0 left, +4 right only, shared by construction/reset/teardown.
// E1B4Node and EventScriptByteTreeNode: +0 left, +4 right, +8 parent/color,
// +C key/padding, +10 stored value. Low parent bit is masked by E1B4Next.
// EventScriptByteTreePair starts at NODE+0xC: key+0, value+4.
// E1B4Tree / ScriptByteMapView: +0 count, +4 root, +8 first iterator; on empty,
// first points at the root SLOT, not the root's value. ScriptState's
// objectTreeFirst/resourceTreeFirst fields at +478/+498 address this first-iterator
// word; actual roots are +474/+494.
// Keyed model map (+470) owns event wrappers; resource map (+490) retains faces;
// shared handler map s_scriptByteKeyMap owns links but borrows function values.
// Node free helpers only release links. Destroy payloads first, then walk links
// left/right/root, then publish {0,0,&root}; never pass the sentinel to free.
typedef char RuntimeTree_node_value_offset[((int)&((E1B4Node*)0)->item == 0x10) ? 1 : -1];
typedef char RuntimeTree_first_offset[((int)&((E1B4Tree*)0)->begin == 8) ? 1 : -1];

// Five callback queues: camera, model, effect, message, and sound.
struct ScriptQueueView {
    char reserved_000[0x434];
    E1B4Vec cameraEvents;
    E1B4Vec modelEvents;
    E1B4Vec effectEvents;
    E1B4Vec messageEvents;
    E1B4Vec soundEvents;
    E1B4Tree keyedModelEvents;
};
extern void* scriptOpcodeHandlers[];

struct C58CCallback {
    virtual ~C58CCallback();
    virtual int check(unsigned int);
};
struct C58CItem {
    unsigned int elapsedTicks; // Elapsed polling ticks, saturated at UINT_MAX; not a lookup key.
    C58CCallback* obj;
    char pad[0x18];
    virtual ~C58CItem();
};
struct C58CVec {
    C58CItem** data;
    unsigned int size, cap;
};
// Delete all owned queued event wrappers and clear the queue size.
static inline void C58CClear(C58CVec* v) {
    C58CItem** p = v->data;
    C58CItem** e = p + v->size;
    if (p != e) {
        for (; p != e; ++p) {
            if (*p) {
                delete *p;
                *p = 0;
            }
        }
    }
    v->size -= ((char*)v->data + v->size) - (char*)v->data;
}
// Poll callbacks with saturated ticks; delete completed callbacks and clear the queue when all finish.
static inline void C58CSweep(C58CVec* v) {
    C58CItem** p;
    unsigned int n;
    C58CItem* it;
    unsigned int completedCount = 0;
    C58CItem** e;
    e = v->data + v->size;
    n = *(volatile unsigned int*)&v->size;
    p = v->data;
    if (p != e) {
        for (; p != e; ++p) {
            it = *p;
            int slotComplete;
            if (!it->obj)
                slotComplete = 1;
            else if (!it->obj->check(it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = it->elapsedTicks + 1;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    delete it->obj;
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
        }
    }
    if (completedCount == n) {
        p = v->data;
        C58CItem** e2 = p + v->size;
        if (p != e2) {
            for (; p != e2; ++p) {
                if (*p) {
                    delete *p;
                    *p = 0;
                }
            }
        }
        v->size -= ((char*)v->data + v->size) - (char*)v->data;
    }
}

// Poll and retire callbacks using the same completion rule with the original declaration order.
static inline void C58CSweepB(C58CVec* v) {
    C58CItem** e;
    C58CItem** p;
    C58CItem* it;
    unsigned int completedCount = 0;
    unsigned int n;
    C58CItem** e2;
    e = v->data + v->size;
    n = *(volatile unsigned int*)&v->size;
    p = v->data;
    if (p != e) {
        for (; p != e; ++p) {
            it = *p;
            int slotComplete;
            if (!it->obj)
                slotComplete = 1;
            else if (!it->obj->check(it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = it->elapsedTicks + 1;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    delete it->obj;
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
        }
    }
    if (completedCount == n) {
        p = v->data;
        e2 = p + v->size;
        if (p != e2) {
            for (; p != e2; ++p) {
                if (*p) {
                    delete *p;
                    *p = 0;
                }
            }
        }
        v->size -= ((char*)v->data + v->size) - (char*)v->data;
    }
}

// Poll a callback vector and delete its wrappers once every callback has completed.
static inline void E1B4Sweep(E1B4Vec* v0) {
    C58CVec* v = (C58CVec*)v0;
    C58CItem** e;
    C58CItem** p;
    unsigned int completedCount = 0;
    C58CItem* it;
    unsigned int n;
    e = v->data + v->size;
    n = *(volatile unsigned int*)&v->size;
    p = v->data;
    if (p != e) {
        for (; p != e; ++p) {
            it = *p;
            int slotComplete;
            if (!it->obj)
                slotComplete = 1;
            else if (!it->obj->check(it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = it->elapsedTicks + 1;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    delete it->obj;
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
        }
    }
    if (completedCount == n) {
        p = v->data;
        C58CItem** e2 = p + v->size;
        if (p != e2) {
            for (; p != e2; ++p) {
                if (*p) {
                    delete *p;
                    *p = 0;
                }
            }
        }
        v->size -= ((char*)v->data + v->size) - (char*)v->data;
    }
}
// Retained unused callback-sweep form for the explicit vtable layout.
static inline void E1B4SweepOld(E1B4Vec* v) {
    unsigned int n;
    E1B4Item* it;
    E1B4Item** p;
    unsigned int completedCount = 0;
    E1B4Item** e = v->data + *(volatile unsigned int*)&v->size;
    n = v->size;
    p = v->data;
    if (p != e) {
        for (; p != e; ++p) {
            it = *p;
            int slotComplete;
            E1B4Obj* ob = it->obj;
            if (!ob)
                slotComplete = 1;
            else if (!((int (*)(E1B4Obj*, unsigned int))(*(void***)ob)[3])(ob, it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = it->elapsedTicks + 1;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    if (it->obj)
                        it->obj->vt->dtor(it->obj, 1);
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
        }
    }
    if (completedCount == n) {
        p = v->data;
        e = p + v->size;
        if (p != e) {
            for (; p != e; ++p) {
                E1B4Sub* x = (E1B4Sub*)*p;
                if (x) {
                    if (x)
                        x->vt->dtor(x, 1);
                    *p = 0;
                }
            }
        }
        unsigned int s = *(volatile unsigned int*)&v->size;
        v->size = s - s;
    }
}

// Advance an ordered-tree iterator, masking the color bit from its parent link.
static inline E1B4Node* E1B4Next(E1B4Node* n) {
    if (n->right) {
        n = n->right;
        while (n->left)
            n = n->left;
        return n;
    }
    E1B4Node* q = (E1B4Node*)(n->parent & ~1u);
    while (n != q->left) {
        n = q;
        q = (E1B4Node*)(n->parent & ~1u);
    }
    return q;
}

// Free the two child subtrees and then the current tree node.
static inline void E1B4Free(E1B4Tree* t, E1B4Node* c) {
    if (c->left)
        ScriptByteMap_FreeSubtree(t, c->left);
    if (c->right)
        ScriptByteMap_FreeSubtree(t, c->right);
    appFreeUni__FPv(c);
}

// Run opcodes until a handler waits, then poll event queues, keyed moves, attached effects, and faces.
int EventScript_RunAndSweep(void* object);
#undef tree
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Advance the tree iterator in place, masking the color bit from its parent link.
static inline void C58CNext(E1B4Node*& n) {
    E1B4Node* p = n->right;
    if (p) {
        while (p->left)
            p = p->left;
        n = p;
    } else {
        // 0x8009C58C: ascend until arriving from a left child.
        while ((p = (E1B4Node*)(n->parent & ~1u)), n != p->left) {
            n = p;
        }
        n = p;
    }
}
// Poll queued events and keyed moves without dispatching new commands, then update effects and faces.
void EventScript_SweepPendingCallbacks(void* object);
#undef tree
#pragma pop

union U4_80093B00 {
    unsigned char c;
    unsigned int w;
};
struct Node_80093B00 {
    void** table;
    unsigned int data;
    // Construct the named model-motion event from the borrowed encoded command.
    Node_80093B00(void* p) {
        ((CEventModelSC_MDL_MOT*)this)->Init((ns0::ModelMotionCommand*)p);
    }
};
struct Interface_80093B00 {
    virtual ~Interface_80093B00();
    virtual int step(unsigned int);
};
struct Fn80093B00State {
    unsigned char pad0[2];
    unsigned short flags;
    char pad4[0x404];
    unsigned int prefixStorageTag;
    char pad40C[4];
    char* prefix;
    char pad414[4];
    unsigned char* cursor;
    unsigned int elapsedWaitTicks;
    char pad420[4];
    Node_80093B00* node;
    char pad428[0x18];
    void** items;
    unsigned int size, capacity;
};
// This command view aliases the common interpreter, not an independent object:
// prefixStorageTag == prefixFlags, prefix == prefixHeap, elapsedWaitTicks == elapsedCommandTicks (elapsed wait
// ticks). node is the owned pending model event at +424. items/size/capacity are
// the model queue at +440.
typedef char Motion_prefix_tag_alias
    [(int)&((Fn80093B00State*)0)->prefixStorageTag == (int)&((EventScriptStateView*)0)->prefixFlags ? 1 : -1];
typedef char
    Motion_prefix_data_alias[(int)&((Fn80093B00State*)0)->prefix == (int)&((EventScriptStateView*)0)->prefixHeap ? 1
                                                                                                                 : -1];
typedef char
    Motion_cursor_alias[(int)&((Fn80093B00State*)0)->cursor == (int)&((EventScriptStateView*)0)->cursor ? 1 : -1];
typedef char Motion_wait_ticks_alias
    [(int)&((Fn80093B00State*)0)->elapsedWaitTicks == (int)&((EventScriptStateView*)0)->elapsedCommandTicks ? 1 : -1];
typedef char Motion_pending_offset[(int)&((Fn80093B00State*)0)->node == 0x424 ? 1 : -1];
typedef char
    Motion_model_queue_alias[(int)&((Fn80093B00State*)0)->items == (int)&((ScriptQueueView*)0)->modelEvents ? 1 : -1];

// MSL range overload: [first,last) is borrowed; destination copies it. The last
// argument transports one allocator byte, never owned storage. A wrapper changes
// argument scheduling here, so give the ABI a type instead of hiding it in a call.
typedef int (*MotionDiagnosticRangeFunction)(void*, unsigned int, unsigned int, const char*, const char*, void*);
struct Str_80093B00 {
    unsigned int w0, len, heapAddressOrInlineTail;
    // Release string storage if its tag selects an owned heap buffer.
    ~Str_80093B00() {
        ((NString*)(this))->~NString();
    }
    // Read byte length from the tagged small-string or heap representation.
    unsigned int size() {
        return (w0 >> 31) == 0 ? (*(unsigned char*)&w0 & 0x7F) : len;
    }
};
struct Item_80093B00 {
    unsigned int elapsedTicks;
    void* callback;
    Str_80093B00 eventName, commandName;
    void* vtable;
    // Wrap the pending event and copy event/opcode names for asynchronous diagnostics.
    Item_80093B00(Fn80093B00State* self, unsigned char* cursor) {
        register const char* kind = conteCommandNames[AsEventScriptCommandView(cursor)->opcode];
        void* p = self->node;
        register const char* prefix = !(AsEventScriptStateView(self)->prefixFlags >> 31)
                                          ? ((ScriptPrefixView*)self)->inlinePrefix
                                          : AsEventScriptStateView(self)->prefixHeap;
        vtable = __vt__15CEventConteProc;
        ClearScriptDiagnosticString128(&eventName);
        ClearScriptDiagnosticString128(&commandName);
        elapsedTicks = 0;
        callback = p;
        // Each call gets its own allocator byte. The volatile reads preserve
        // compiler ABI scratch traffic; these bytes have no gameplay identity.
        // They are not initialization flags, string lengths or transferred owners.
        volatile U4_80093B00 eventAllocatorScratch;
        unsigned int oldEventNameLength = eventName.size();
        U4_80093B00 eventAllocatorArgument;
        eventAllocatorArgument.c = eventAllocatorScratch.c;
        ((MotionDiagnosticRangeFunction)String_ReplaceRange)(&eventName, 0, oldEventNameLength, prefix,
                                                             prefix + strlen(prefix), &eventAllocatorArgument);
        volatile U4_80093B00 commandAllocatorScratch;
        unsigned int oldCommandNameLength = commandName.size();
        U4_80093B00 commandAllocatorArgument;
        commandAllocatorArgument.c = commandAllocatorScratch.c;
        ((MotionDiagnosticRangeFunction)String_ReplaceRange)(&commandName, 0, oldCommandNameLength, kind,
                                                             kind + strlen(kind), &commandAllocatorArgument);
    }
};
struct ErrBase_80093B00 {
    void* table;
    char* data;
    MessageControl* owner;
    // Copy diagnostic text and transfer its array to a shared exception control block.
    ErrBase_80093B00(register const char* text) {
        U4_80093B00 z;
        z.c = 0;
        table = (void*)__vt__Q23std11logic_error;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8009D25C guard(p, &z);
            owner = new MessageControl(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Invoke the standard exception-base destructor through the runtime ABI.
    ~ErrBase_80093B00() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
namespace std {
struct length_error_80093B00 : ErrBase_80093B00 {
    // Install the length-error vtable after constructing owned diagnostic text.
    length_error_80093B00(const char* p) : ErrBase_80093B00(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    // Run the runtime cleanup for the exception character-array owner.
    ~length_error_80093B00() {
        __dt__Q33std6detail16char_array_guardFv(this, 0);
    }
};
}
// Keep this caller-scope exception lifetime explicit for code matching.
#define LE_THROW_80093B00                                                                                        \
    {                                                                                                            \
        std::length_error_80093B00 ex(bpPlayerResourceStrings +                                                  \
                                      SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));           \
        __throw(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName), &ex, \
                __dt__Q23std12length_errorFv);                                                                   \
    }
} // extern "C"

// Game entry points in original address order.
extern "C" {

#pragma push
#pragma use_lmw_stmw on
// Create the model-face event and either poll it synchronously or queue its owned wrapper.
int CEventConteProc::ProcessModelFaceCommand() {
    Fn80092804Arg* self = (Fn80092804Arg*)this;
    // Phase 1: skip or construct the face event, then poll if synchronous.
    unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    if (!self->count)
        self->object = new Fn80092804Node(cursor);
    if (!cursor[11]) {
        if (!((int (*)(void*, unsigned int))(*(void***)self->object)[3])(self->object, self->count)) {
            if (self->count < kSaturatedCommandTick)
                ++self->count;
            return 0;
        }
        if (self->object) {
            if (self->object)
                ((void (*)(void*, int))(*(void***)self->object)[2])(self->object, 1);
            self->object = 0;
        }
    } else {
        Item_80092804* item = new Item_80092804(self, cursor);
        if (self->size < self->capacity)
            self->items[self->size++] = (void*)item;
        else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));
                exception_runtime::__throw(exception_runtime::bpPlayerResourceStrings +
                                               SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName),
                                           error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            VecBuf_80097998 buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));
                exception_runtime::__throw(exception_runtime::bpPlayerResourceStrings +
                                               SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName),
                                           error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_80097998(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_80097998(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, vectorLengthError));
                exception_runtime::__throw(exception_runtime::bpPlayerResourceStrings +
                                               SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, lengthErrorTypeName),
                                           error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            buf.first[buf.count + sz] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}
#pragma pop

// Run the model-head event until complete, or transfer it to the model callback queue.
int CEventConteProc::ProcessModelHeadCommand() {
    Fn80092804Arg* self = (Fn80092804Arg*)this;
    // Phase 1: skip or construct the head event, then poll if synchronous.
    unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    if (!self->count)
        self->object = new Fn80092E58Node(cursor);
    if (!cursor[11]) {
        Interface_80092E58* obj = (Interface_80092E58*)self->object;
        if (!obj->check(self->count)) {
            if (self->count < 0xffffffffu)
                ++self->count;
            return 0;
        }
        if (self->object) {
            delete (Interface_80092E58*)self->object;
            self->object = 0;
        }
    } else {
        Item_80092E58* item = new Item_80092E58(self, cursor);
        if (self->size < self->capacity) {
            ++self->size;
            self->items[self->size - 1] = item;
        } else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1)
                length_error_80092E58();
            unsigned int growSmall;
            unsigned int growLarge;
            unsigned int minGrow = self->size + 1 - self->capacity;
            Vec_80092E58 buf(&self->capacity);
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                length_error_80092E58();
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                growSmall = ((cap + 1) * 3) / 5;
                newCap = cap + max_80097998(growSmall, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                growLarge = (cap + 1) >> 1;
                newCap = cap + max_80097998(growLarge, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                length_error_80092E58();
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int oldSize = self->size;
            buf.data = data;
            buf.oldSize = oldSize;
            (buf.data + oldSize)[buf.size] = (unsigned int)item;
            buf.capacity = newCap;
            buf.size = buf.size + 1;
            unsigned int* old = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - old;
            buf.oldSize = oldSize - n;
            memcpy(data + buf.oldSize, old, n * 4);
            memset(old, 0, n * 4);
            unsigned int* next;
            unsigned int nextCap;
            unsigned int total;
            unsigned int oldCap;
            total = buf.size + n;
            buf.size = total;
            next = buf.data;
            oldCap = self->capacity;
            nextCap = buf.capacity;
            self->size = 0;
            buf.data = (unsigned int*)self->items;
            self->capacity = nextCap;
            buf.capacity = oldCap;
            self->items = (void**)next;
            self->size = total;
            buf.size = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}

// Run the head-reset event until complete, or transfer it to the model callback queue.
int CEventConteProc::ProcessModelHeadResetCommand() {
    Fn800934ACArg* self = (Fn800934ACArg*)this;
    // Phase 1: skip or construct the head-reset event, then wrap for the model queue.
    unsigned char* cursor = self->cursor;
    if (self->value != 0) {
        self->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    if (self->count == 0)
        self->object = new Fn800934ACNode(cursor);
    if (cursor[11] == 0) {
        void* obj = self->object;
        if (!((int (*)(void*, unsigned int))(*(void***)obj)[3])(obj, self->count)) {
            if (self->count < kSaturatedCommandTick)
                ++self->count;
            return 0;
        }
        if (self->object) {
            if (self->object)
                ((void (*)(void*, int))(*(void***)self->object)[2])(self->object, 1);
            self->object = 0;
        }
    } else {
        Item_800934AC* item = new Item_800934AC(self, cursor);
        if (self->size < self->capacity)
            self->items[self->size++] = (void*)item;
        else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1)
                LE_THROW_800934AC
            VecBuf_800934AC buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                LE_THROW_800934AC
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_800934AC(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_800934AC(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                LE_THROW_800934AC
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            buf.first[buf.count + sz] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Run the named model-motion event until complete, or transfer it to the model callback queue.
// Name starts at +8; end points just past its NUL/padding. *end is the async
// byte in a four-byte trailer, so next command is end+4. The other three trailer
// bytes are skipped here. Waiting leaves cursor unchanged and saturates elapsedWaitTicks.
int CEventConteProc::ProcessModelMotionCommand() {
    Fn80093B00State* self = (Fn80093B00State*)this;
    register unsigned char* cursor = self->cursor;
    register char* text = (char*)cursor + kMotionNameOffset;
    unsigned int offset = (strlen(text) + 4) & ~3u;
    register unsigned char* end = (unsigned char*)text + offset;
    register unsigned char next = *end;
    if (self->flags) {
        self->cursor = end + kMotionTrailerBytes;
        return 1;
    }
    // Phase 1: construct once from the borrowed command bytes. Zero elapsedWaitTicks
    // means the event has not yet been constructed.
    if (self->elapsedWaitTicks == 0)
        self->node = new Node_80093B00(cursor);
    if (next == 0) {
        if (((Interface_80093B00*)self->node)->step(self->elapsedWaitTicks) == 0) {
            if (self->elapsedWaitTicks < kSaturatedCommandTick)
                ++self->elapsedWaitTicks;
            return 0;
        }
        if (self->node) {
            delete (Interface_80093B00*)self->node;
            self->node = 0;
        }
    } else {
        // Phase 2 (async): wrap the event, append/transfer it, then disown node.
        Item_80093B00* item = new Item_80093B00(self, cursor);
        if (self->size < self->capacity) {
            unsigned int n = self->size + 1;
            self->size = n;
            self->items[n - 1] = item;
        } else {
            if (kMaxPointerVectorSize - self->capacity < 1)
                LE_THROW_80093B00
            VecBuf_800967A4 buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                LE_THROW_80093B00
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_800967A4(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_800967A4(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                LE_THROW_80093B00
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            (buf.first + sz)[buf.count] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        self->node = 0;
    }
    self->cursor = end + kMotionTrailerBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_common_subs off
// Replace an existing keyed model move, then wait or transfer it to the keyed callback map.
int CEventConteProc::ProcessModelMoveCommand() {
    Fn8009416CArg* self = (Fn8009416CArg*)this;
    // Phase 1: skip or replace the previous byte-keyed callback before creating
    // the new move event. Tree-node cleanup here does not advance the command.
    unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kModelMoveCommandBytes;
        return 1;
    }
    if (self->index == 0) {
        Fn8009416CNode* p = (Fn8009416CNode*)self->treeRoot;
        Fn8009416CNode* best = (Fn8009416CNode*)&self->treeRoot;
        while (p) {
            if (p->key >= cursor[kModelMoveKeyOffset]) {
                best = p;
                p = p->left;
            } else
                p = p->right;
        }
        if (best == (Fn8009416CNode*)&self->treeRoot || cursor[kModelMoveKeyOffset] < best->key)
            best = (Fn8009416CNode*)&self->treeRoot;
        if (best != (Fn8009416CNode*)&self->treeRoot) {
            void* old = best->value;
            if (old) {
                if (old)
                    ((void (*)(void*, int))(((ScriptOwnedEventView*)old)->vtable)[2])(old, 1);
                best->value = 0;
            }
            Fn8009416CNode* removed = best;
            if (best == (Fn8009416CNode*)self->treeCache) {
                Fn8009416CNode* q = best->right;
                if (q) {
                    Fn8009416CNode* nxt;
                    while ((nxt = q->left) != 0)
                        q = nxt;
                    best = q;
                } else {
                    Fn8009416CNode* par;
                    while (best != (par = (Fn8009416CNode*)(best->parent & ~1u))->left)
                        best = par;
                    best = par;
                }
                self->treeCache = best;
            }
            RBTree_RebalanceAfterErase(removed, self->treeRoot);
            appFreeUni__FPv(removed);
            --self->count;
        }
        self->pending = new Fn8009416CPending(cursor);
    }
    // Phase 2: wait synchronously, or publish a wrapper to the keyed model map.
    if (cursor[kModelMoveAsyncOffset] == 0) {
        if (!((Fn8009416CBase*)self->pending)->update(self->index)) {
            if (self->index < kSaturatedCommandTick)
                self->index = self->index + 1;
            return 0;
        }
        Fn8009416CBase* p = (Fn8009416CBase*)self->pending;
        if (p) {
            if (p)
                p->destroy(1);
            self->pending = 0;
        }
    } else {
        Fn8009416CItem* item = new Fn8009416CItem(self, cursor);
        struct {
            unsigned int key;
            unsigned int value;
        } keyValue;
        void* predecessor;
        unsigned char updateBegin, left;
        void* found =
            EventFxMap_FindInsertionPosition((char*)&((ScriptObjectMapView*)self)->objects,
                                             cursor + kOneWordCommandBytes, &predecessor, &left, &updateBegin);
        char* slot = (char*)&AsEventScriptByteTreeNode(predecessor)->key;
        if (!predecessor || ((Fn8009416CNode*)predecessor)->key < cursor[kModelMoveKeyOffset]) {
            *(unsigned char*)&keyValue.key = cursor[kModelMoveKeyOffset];
            keyValue.value = s_scriptByteMapDefaultValue;
            slot = (char*)EventFxMap_InsertNode((char*)&((ScriptObjectMapView*)self)->objects, (unsigned int)found,
                                                left, updateBegin, &keyValue) +
                   SCRIPT_MEMBER_OFFSET(EventScriptByteTreeNode, key);
        }
        *(void**)(slot + 4) = item;
        self->pending = 0;
    }
    self->cursor = cursor + kModelMoveCommandBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Run the model-rotation event until complete, or transfer it to the model callback queue.
int CEventConteProc::ProcessModelRotationCommand() {
    Fn800944F0Arg* self = (Fn800944F0Arg*)this;
    // Phase 1: skip or construct the rotation event, then wrap for the model queue.
    unsigned char* cursor = self->cursor;
    if (self->value != 0) {
        self->cursor = cursor + 16;
        return 1;
    }
    if (self->count == 0)
        self->object = new Fn800944F0Node(cursor);
    if (cursor[12] == 0) {
        void* obj = self->object;
        if (!((int (*)(void*, unsigned int))(*(void***)obj)[3])(obj, self->count)) {
            if (self->count < kSaturatedCommandTick)
                ++self->count;
            return 0;
        }
        if (self->object) {
            if (self->object)
                ((void (*)(void*, int))(*(void***)self->object)[2])(self->object, 1);
            self->object = 0;
        }
    } else {
        Item_800944F0* item = new Item_800944F0(self, cursor);
        if (self->size < self->capacity)
            self->items[self->size++] = (void*)item;
        else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1)
                LE_THROW_80094F20
            VecBuf_80094F20 buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                LE_THROW_80094F20
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_80094F20(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_80094F20(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                LE_THROW_80094F20
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            buf.first[buf.count + sz] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + 16;
    return 1;
}
#pragma pop

// SC_FACE_PARTS: send the character/channel bytes and two word operands to facial-part control.
int CEventConteProc::ProcessFacePartsCommand(Fn80094B44Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    ConteResource_SetCategoryResourcePair(cursor[2], cursor[3], AsEventScriptCommandView(cursor)->operandWord0,
                                          AsEventScriptCommandView(cursor)->operandWord1);
    script->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}

// Find or create a face object, resolve the nested script tables, and run its subscript to completion.
int CEventConteProc::ExecuteFaceSubscript(void* script) {
    void* obj = script;
    unsigned char* cursor = AsEventScriptStateView(obj)->cursor;
    unsigned short v = EventScript_GetConditionFlags((Fn8009BEA4Arg*)obj, 0);
    if (v != 0) {
        AsEventScriptStateView(obj)->cursor = cursor + kOneWordCommandBytes;
        return 1;
    }
    // Phase 1: reuse or create the face object keyed by command byte +1.
    // Resource-map values retain the face; this handler only borrows it while
    // running the nested interpreter. Its temporary pending alias is later cleared.
    void* faceIterator = 0;
    void* faceEndIterator = 0;
    void* faceMapNode = ScriptMapIterator_GetNode((void**)((char*)&((ScriptResourceMapView*)obj)->resources));
    if (faceMapNode != 0) {
        ScriptResourceMap_Find(&faceIterator, (void*)((char*)&((ScriptResourceMapView*)obj)->resources), cursor + 1);
        ScriptResourceTree_GetEndIterator(&faceEndIterator, (void*)((char*)&((ScriptResourceMapView*)obj)->resources));
        int cmp = ScriptTreeIterator_NotEqual(&faceIterator, &faceEndIterator);
        if (cmp != 0) {
            void* p = ((CEventHandleTreeIterator*)&faceIterator)->GetValue();
            AsEventScriptStateView(obj)->pendingObject = (void*)AsEventScriptByteTreePair(p)->value;
        }
    }
    if (AsEventScriptStateView(obj)->pendingObject == 0) {
        void* nw = __nw__FUl(0x34);
        void* newFace = nw;
        if (nw != 0) {
            ((CEventFace*)nw)->__ct();
        }
        AsEventScriptStateView(obj)->pendingObject = newFace;
        void* ins =
            ScriptByteMap_GetOrInsertValue((void*)((char*)&((ScriptResourceMapView*)obj)->resources), cursor + 1);
        *(void**)ins = newFace;
    }
    void* slots = CourseSlotTable_GetInstance();
    void* slotRecord = SlotTable_GetEntry(slots, AsEventScriptCommandView(cursor)->operandByte);
    void* slotRangeResultAbi = CourseSlotTable_IsExtraSlotRange(slots, AsEventScriptCommandView(cursor)->operandByte);
    ((CEventFace*)(AsEventScriptStateView(obj)->pendingObject))->SelectModelAndTrack(cursor);
    // Phase 2: four nested word tables, indexed in this order:
    // command[2] -> low byte of slot-range result -> slotRecord[0] -> command[4].
    // EVERY loaded word is byte-swapped and relative to the ORIGINAL table base,
    // never relative to its containing table. Only the table INDEX is a byte;
    // the resulting offsets/addresses are full 32-bit words. No bounds checks.
    // +49C is published from CStoryConte's eventDataLoad->buffer in story_conte_cutscene.cpp
    // before playback; it is separate from the active script argument at +414.
    unsigned int tableBaseAddress = AsEventScriptStateView(obj)->faceScriptTableBase;
    unsigned int commandTableOffset = WordArray_LoadByteSwapped((unsigned int*)tableBaseAddress, cursor[2]);
    unsigned int commandTableAddress = tableBaseAddress + commandTableOffset;
    // The legacy declaration carries the integer 0/1 result in a pointer-shaped
    // r3 value; only its low byte is consumed, never dereference it.
    unsigned int rangeTableOffset =
        WordArray_LoadByteSwapped((unsigned int*)commandTableAddress, (unsigned char)(unsigned int)slotRangeResultAbi);
    unsigned int rangeTableAddress = tableBaseAddress + rangeTableOffset;
    unsigned char slotEntryByte = *(unsigned char*)slotRecord;
    unsigned int slotTableOffset = WordArray_LoadByteSwapped((unsigned int*)rangeTableAddress, slotEntryByte);
    unsigned int slotTableAddress = tableBaseAddress + slotTableOffset;
    unsigned char subscriptIndex = cursor[4];
    unsigned int subscriptOffset = WordArray_LoadByteSwapped((unsigned int*)slotTableAddress, subscriptIndex);
    unsigned int subscriptAddress = tableBaseAddress + subscriptOffset;
    void* nw2 = __nw__FUl(0x4A8);
    void* nestedInterpreter = nw2;
    if (nw2 != 0) {
        {
            try {
                ((CEventHandle*)nw2)
                    ->__ct(bpPlayerResourceStrings + SCRIPT_MEMBER_OFFSET(ScriptResourceStrings, facialData),
                           (void*)tableBaseAddress, subscriptAddress);
            } catch (...) {
                __dl__FPv(nw2);
                throw;
            }
        }
    }
    // Phase 3: lend the face to the child, run synchronously, then clear its alias
    // before deleting the child. There is no frame yield or timeout in this loop.
    AsEventScriptStateView(nestedInterpreter)->pendingObject = AsEventScriptStateView(obj)->pendingObject;
    for (;;) {
        int res = EventScript_RunAndSweep(nestedInterpreter);
        if (res != 0)
            break;
    }
    (*(unsigned int*)&AsEventScriptStateView(nestedInterpreter)->pendingObject) = 0;
    if (nestedInterpreter != 0) {
        if (nestedInterpreter != 0) {
            void** vt = AsEventScriptStateView(nestedInterpreter)->eventHandleVtable;
            ((void (*)(void*, int))vt[2])(nestedInterpreter, 1);
        }
    }
    AsEventScriptStateView(obj)->pendingObject = 0;
    AsEventScriptStateView(obj)->cursor = cursor + kOneWordCommandBytes;
    return 1;
}

#pragma push
#pragma opt_common_subs off
// SC_FACE_STOP: stop both facial animation channels for the selected byte key.
int CEventConteProc::ProcessFaceStopCommand() {
    Fn80094D74Arg* self = (Fn80094D74Arg*)this;
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    Fn80094D74Clear(self, AsEventScriptCommandView(cursor)->operandByte, 0);
    Fn80094D74Clear(self, AsEventScriptCommandView(cursor)->operandByte, 1);
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// SC_FACE_TIME: apply facial timing operands to the interpreter's required +48C face alias.
int CEventConteProc::ProcessFaceTimeCommand(Fn80094EB8Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    ((CEventFace*)(script->object))->AppendKeyframe(cursor);
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Run the effect-fade event until complete, or transfer it to the effect callback queue.
// Poll a borrowed fade callback through its recovered update slot; zero means still pending.
static inline int PollEffectFadeCallback128(void* callback, unsigned int elapsedTicks) {
    return ((int (*)(void*, unsigned int))(*(void***)callback)[3])(callback, elapsedTicks);
}

// Delete a completed interpreter-owned fade and clear its pending alias.
static inline void ReleaseCompletedEffectFade128(Fn80094F20Arg* self) {
    if (self->object) {
        if (self->object)
            ((void (*)(void*, int))(*(void***)self->object)[2])(self->object, 1);
        self->object = 0;
    }
}

// Complete a synchronous fade or publish its callback to the owning asynchronous queue.
int CEventConteProc::ProcessEffectFadeCommand() {
    Fn80094F20Arg* self = (Fn80094F20Arg*)this;
    // Phase 1: skip or construct the fade event, then wrap for the effect queue.
    unsigned char* cursor = self->cursor;
    if (self->value != 0) {
        self->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    if (self->count == 0)
        self->object = new Fn80094F20Node(cursor);
    // Sync path: the interpreter retains the callback while polling returns zero.
    if (cursor[8] == 0) {
        void* obj = self->object;
        if (!PollEffectFadeCallback128(obj, self->count)) {
            if (self->count < kSaturatedCommandTick)
                ++self->count;
            return 0;
        }
        ReleaseCompletedEffectFade128(self);
    } else {
        // Async path: package once, publish to the effect queue, then relinquish pending.
        Item_80094F20* item = new Item_80094F20(self, cursor);
        if (self->size < self->capacity)
            self->items[self->size++] = (void*)item;
        else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1)
                LE_THROW_80094F20
            VecBuf_80094F20 buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                LE_THROW_80094F20
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_80094F20(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_80094F20(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                LE_THROW_80094F20
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            buf.first[buf.count + sz] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}
#pragma pop

// Resolve an actor bone, scene object, or default source and attach the named effect at its offset.
int CEventConteProc::ExecuteAttachedEffectCommand() {
    Fn80095574Arg* self = (Fn80095574Arg*)this;
    unsigned char* cursor = self->cursor;
    char* attachmentName = (char*)cursor + kTwoWordCommandBytes;
    unsigned int attachmentLength = (unsigned int)strlen(attachmentName);
    char* effectName = attachmentName + ((attachmentLength + 4) & ~3);
    if (self->flags != 0) {
        self->cursor = (unsigned char*)(effectName + ((strlen(effectName) + 4) & ~3));
        return 1;
    }
    float offsetY;
    float offsetZ;
    offsetZ = (float)*(short*)(cursor + 6);
    offsetY = (float)(*(short*)&AsEventScriptCommandView(cursor)->operandWord0);
    float offsetX = (float)AsEventScriptCommandView(cursor)->operandShort;
    float offset[3];
    offset[1] = offsetY;
    offset[2] = offsetZ;
    offset[0] = offsetX;
    if (cursor[11] < kActorSlotCount) {
        if (((unsigned int&)GetConteModelSlotsSlot()) == 0) {
            if (s_courseSlotTableInitialized == 0) {
                CourseSlotTable_InitEmpty(s_conteModelSlots);
                s_courseSlotTableInitialized = 1;
            }
            ((unsigned int&)GetConteModelSlotsSlot()) = (unsigned int)s_conteModelSlots;
        }
        void* actorSlot = SlotTable_GetEntry((void*)((unsigned int&)GetConteModelSlotsSlot()), cursor[11]);
        if (strlen(attachmentName) == 0)
            EventFxMap_SetEffect(((ScriptEffectMapView*)self)->effectMap, AsEventScriptCommandView(cursor)->operandByte,
                                 effectName, ((ActorSlotView*)actorSlot)->puppet, offset);
        else if (WPuppet_FindNamedBoneEntry(((ActorSlotView*)actorSlot)->puppet, attachmentName))
            EventFxMap_ReplaceFromNamedBone(((ScriptEffectMapView*)self)->effectMap,
                                            AsEventScriptCommandView(cursor)->operandByte, effectName,
                                            ((ActorSlotView*)actorSlot)->puppet, attachmentName, offset);
        else if (WPuppet_FindOrCacheBone(((ActorSlotView*)actorSlot)->puppet, attachmentName))
            EventFxMap_SetBoneAttachedEffect(((ScriptEffectMapView*)self)->effectMap,
                                             AsEventScriptCommandView(cursor)->operandByte, effectName,
                                             ((ActorSlotView*)actorSlot)->puppet, attachmentName, offset);
    } else if (strlen(attachmentName) != 0) {
        void* sceneObject = NamedEffectList_FindByName(g_sceneManager, attachmentName);
        if (sceneObject)
            EventFxMap_SetEffect(((ScriptEffectMapView*)self)->effectMap, AsEventScriptCommandView(cursor)->operandByte,
                                 effectName, sceneObject, offset);
    } else
        EventFxMap_ReplaceWithDefaultSource(((ScriptEffectMapView*)self)->effectMap,
                                            AsEventScriptCommandView(cursor)->operandByte, effectName, offset);
    self->cursor = (unsigned char*)(effectName + ((strlen(effectName) + 4) & ~3));
    return 1;
}

// SC_EFF_CUT: erase the attached effect with the command byte key.
int EventScript_EraseEffectByKey(Fn80094EB8Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    EventFxMap_EraseByKey(((ScriptEffectMapView*)script)->effectMap, AsEventScriptCommandView(cursor)->operandByte);
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Run a message request, synchronizing face channels with text reveal until completion.
int CEventConteProc::ExecuteMessageRequestCommand() {
    Fn80095808Arg* self = (Fn80095808Arg*)this;
    // Phase 1: skip inactive commands, or create/poll the owned request once.
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kOneWordCommandBytes;
        return 1;
    }
    if (self->index == 0) {
        self->root = new Fn80095808Root(cursor);
    }
    int done = ((Fn80095808Base*)self->root)->update(self->index);
    if (!done) {
        // Phase 2: while waiting, mirror reveal/hold state to face track 1.
        // The request remains interpreter-owned across return 0.
        if (((unsigned int&)Sing_805DC670()) == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
                s_bpPlayerInitialized = 1;
            }
            ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
        }
        void* playerObject = (void*)(void*)((CBpPlayer*)(((unsigned int&)Sing_805DC670())))->GetControlEntry();
        if (playerObject && BpObject_IsCurrentScriptComplete(playerObject)) {
            if (((unsigned int&)Sing_805DC670()) == 0) {
                if (s_bpPlayerInitialized == 0) {
                    ((CBpPlayer*)s_bpPlayer)->__ct();
                    __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
                    s_bpPlayerInitialized = 1;
                }
                ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
            }
            Fn80095808Node* item =
                (Fn80095808Node*)((CMessageOwnerLayout*)((unsigned int&)Sing_805DC670()))->GetOrCreateMessageDraw();
            if (item) {
                if (((CMsgDraw*)item)->IsRevealComplete() || ((CMsgDraw*)item)->revealDelayFrames) {
                    // original +0x6C
                    unsigned char key = AsEventScriptCommandView(cursor)->operandByte;
                    if (self->enabled) {
                        Fn80095808Node* p = (Fn80095808Node*)self->tree;
                        Fn80095808Node* best = (Fn80095808Node*)&self->tree;
                        while (p) {
                            if (p->key >= key) {
                                best = p;
                                p = p->left;
                            } else
                                p = p->right;
                        }
                        if (best == (Fn80095808Node*)&self->tree || key < best->key)
                            best = (Fn80095808Node*)&self->tree;
                        if (best != (Fn80095808Node*)&self->tree)
                            ((CEventFace*)(best->payload))->SetTrackFrameHold(1, (void*)1);
                    }
                } else {
                    unsigned char key = AsEventScriptCommandView(cursor)->operandByte;
                    if (self->enabled) {
                        Fn80095808Node* p = (Fn80095808Node*)self->tree;
                        Fn80095808Node* best = (Fn80095808Node*)&self->tree;
                        while (p) {
                            if (p->key >= key) {
                                best = p;
                                p = p->left;
                            } else
                                p = p->right;
                        }
                        if (best == (Fn80095808Node*)&self->tree || key < best->key)
                            best = (Fn80095808Node*)&self->tree;
                        if (best != (Fn80095808Node*)&self->tree)
                            ((CEventFace*)(best->payload))->SetTrackFrameHold(1, 0);
                    }
                }
            }
        }
        if (self->index < kSaturatedCommandTick)
            self->index = self->index + 1;
        return 0;
    }
    // Phase 3: release the request, finish face track 1 and advance 8 bytes.
    if (self->root) {
        if (self->root)
            ((Fn80095808Base*)self->root)->destroy(1);
        self->root = 0;
    }
    unsigned char key = AsEventScriptCommandView(cursor)->operandByte;
    if (self->enabled) {
        Fn80095808Node* p = (Fn80095808Node*)self->tree;
        Fn80095808Node* best = (Fn80095808Node*)&self->tree;
        while (p) {
            if (p->key >= key) {
                best = p;
                p = p->left;
            } else
                p = p->right;
        }
        if (best == (Fn80095808Node*)&self->tree || key < best->key)
            best = (Fn80095808Node*)&self->tree;
        if (best != (Fn80095808Node*)&self->tree)
            ((CEventFace*)(best->payload))->ApplyLastFrameAndClearTrack(1);
    }
    self->cursor = cursor + kOneWordCommandBytes;
    return 1;
}

// Run the message-window event until complete, or transfer it to the message callback queue.
int CEventConteProc::ProcessMessageWindowCommand() {
    Fn80095B54Arg* self = (Fn80095B54Arg*)this;
    // Phase 1: skip or construct the window event, then wrap for the message queue.
    register unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (!self->count)
        self->object = new Fn80095B54Node(cursor);
    if (!cursor[2]) {
        if (!((Fn80095B54Base*)self->object)->check(self->count)) {
            if (self->count < kSaturatedCommandTick)
                ++self->count;
            return 0;
        }
        if (self->object) {
            if (self->object)
                ((Fn80095B54Base*)self->object)->destroy(1);
            self->object = 0;
        }
    } else {
        Item_80095B54* item = new Item_80095B54(self, cursor);
        if (self->size < self->capacity) {
            unsigned int n = self->size + 1;
            self->size = n;
            self->items[n - 1] = (void*)item;
        } else {
            // Phase 2: reserve a larger pointer buffer, append the new wrapper,
            // transfer old handles in order, then swap storage. The local guard
            // frees the old pointer array, retaining the transferred events.
            if (kMaxPointerVectorSize - self->capacity < 1)
                LE_THROW_80095B54
            VecBuf_80097998 buf(&self->capacity);
            unsigned int minGrow = self->size + 1 - self->capacity;
            unsigned int cap = self->capacity;
            if (minGrow > kMaxPointerVectorSize - cap)
                LE_THROW_80095B54
            unsigned int newCap;
            if (cap < kPointerVectorSmallGrowthLimit) {
                unsigned int g = ((cap + 1) * 3) / 5;
                newCap = cap + max_80097998(g, minGrow);
            } else if (cap < kPointerVectorLargeGrowthLimit) {
                unsigned int g = (cap + 1) >> 1;
                newCap = cap + max_80097998(g, minGrow);
            } else
                newCap = kMaxPointerVectorSize;
            if (newCap > kMaxPointerVectorSize)
                LE_THROW_80095B54
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
            unsigned int sz = self->size;
            buf.first = data;
            buf.used = sz;
            (buf.first + sz)[buf.count] = (unsigned int)item;
            buf.count = buf.count + 1;
            buf.cap = newCap;
            unsigned int* oldItems = (unsigned int*)self->items;
            unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
            buf.used = sz - n;
            memcpy(data + buf.used, oldItems, n * 4);
            memset(oldItems, 0, n * 4);
            unsigned int total = buf.count + n;
            buf.count = total;
            unsigned int* nf = buf.first;
            unsigned int oc = self->capacity;
            unsigned int nc = buf.cap;
            self->size = 0;
            buf.first = (unsigned int*)self->items;
            self->capacity = nc;
            buf.cap = oc;
            self->items = (void**)nf;
            self->size = total;
            buf.count = 0;
        }
        // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
        // The cursor advances only on the completed or transferred path.
        self->object = 0;
    }
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect request event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundSeRequestCommand() {
    Fn800961A8Arg* self = (Fn800961A8Arg*)this;
    // Phase 1: locate the padded name at +4, skip or wrap the SE-request event.
    char* text;
    unsigned char* cursor;
    cursor = self->cursor;
    text = (char*)cursor + kBasicCommandBytes;
    if (self->flags) {
        unsigned int len = strlen(text);
        self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
        return 1;
    }
    if (!self->commandTick)
        self->pendingSound = new Fn800961A8Node(cursor);
    Item_800961A8* item = new Item_800961A8(self, cursor);
    unsigned int sz = self->size;
    if (sz < self->capacity) {
        unsigned int n = sz + 1;
        self->size = n;
        self->items[n - 1] = (void*)item;
    } else {
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_800961A8
        VecBuf_800961A8 buf(&self->capacity);
        unsigned int minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_800961A8
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            unsigned int g = ((cap + 1) * 3) / 5;
            newCap = cap + max_800961A8(g, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            unsigned int g = (cap + 1) >> 1;
            newCap = cap + max_800961A8(g, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_800961A8
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.used = sz;
        buf.first[buf.count + sz] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.used = sz - n;
        memcpy(data + buf.used, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pendingSound = 0;
    unsigned int len = strlen(text);
    self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect play event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundSePlayCommand() {
    Fn800967A4Arg* self = (Fn800967A4Arg*)this;
    // Phase 1: locate the padded name at +4, skip or wrap the SE-play event.
    Item_800967A4* item;
    char* text;
    unsigned char* cursor;
    cursor = self->cursor;
    text = (char*)cursor + kBasicCommandBytes;
    if (self->flags) {
        unsigned int len = strlen(text);
        self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
        return 1;
    }
    if (!self->commandTick)
        self->pendingSound = new Fn800967A4Node(cursor);
    item = new Item_800967A4(self, cursor);
    unsigned int sz0 = self->size;
    if (sz0 < self->capacity) {
        unsigned int n = sz0 + 1;
        self->size = n;
        self->items[n - 1] = (void*)item;
    } else {
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_800967A4
        VecBuf_800967A4 buf(&self->capacity);
        unsigned int minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_800967A4
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            unsigned int g = ((cap + 1) * 3) / 5;
            newCap = cap + max_800967A4(g, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            unsigned int g = (cap + 1) >> 1;
            newCap = cap + max_800967A4(g, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_800967A4
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.used = sz;
        unsigned int* slot = (unsigned int*)((char*)data + 4 * sz);
        slot[buf.count] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.used = sz - n;
        memcpy(data + buf.used, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pendingSound = 0;
    unsigned int len = strlen(text);
    self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect stop event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundSeStopCommand() {
    Fn80096DA0Arg* self = (Fn80096DA0Arg*)this;
    // Phase 1: locate the padded name at +4, skip or wrap the SE-stop event.
    unsigned char* cursor = self->cursor;
    char* text = (char*)cursor + kBasicCommandBytes;
    if (self->flags) {
        unsigned int len = strlen(text);
        self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
        return 1;
    }
    if (!self->commandTick)
        self->pending = new Fn80096DA0Node(cursor);
    Item_80096DA0* item = new Item_80096DA0((Fn80097998Arg*)self, cursor);
    cursor = 0;
    if (self->size < self->capacity) {
        unsigned int n = self->size + 1;
        self->size = n;
        self->items[n - 1] = (void*)item;
    } else {
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_80096DA0
        VecBuf_80097998 buf(&self->capacity);
        unsigned int minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_80096DA0
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            unsigned int g = ((cap + 1) * 3) / 5;
            newCap = cap + max_80097998(g, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            unsigned int g = (cap + 1) >> 1;
            newCap = cap + max_80097998(g, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_80096DA0
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.used = sz;
        buf.first[buf.count + sz] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.used = sz - n;
        memcpy(data + buf.used, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pending = 0;
    unsigned int len = strlen(text);
    self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Create a named sound-effect pitch event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundSePitchCommand() {
    Fn8009739CArg* self = (Fn8009739CArg*)this;
    // Phase 1: locate the padded name at +12, skip or wrap the SE-pitch event.
    register unsigned char* cursor = self->cursor;
    register unsigned char* text = cursor + kSePitchNameOffset;
    if (self->flags) {
        unsigned int len = strlen((char*)text);
        self->cursor = text + ((len + 4) & ~3);
        return 1;
    }
    if (!self->commandTick)
        self->pending = new Fn8009739CNode(cursor);
    Item_8009739C* item = new Item_8009739C(self, cursor);
    if (self->size < self->capacity) {
        unsigned int n = self->size + 1;
        self->size = n;
        self->items[n - 1] = (void*)item;
    } else {
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_8009739C
        VecBuf_800967A4 buf(&self->capacity);
        unsigned int minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_8009739C
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            unsigned int g = ((cap + 1) * 3) / 5;
            newCap = cap + max_800967A4(g, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            unsigned int g = (cap + 1) >> 1;
            newCap = cap + max_800967A4(g, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_8009739C
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.used = sz;
        (buf.first + sz)[buf.count] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.used = sz - n;
        memcpy(data + buf.used, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pending = 0;
    unsigned int len = strlen((char*)text);
    self->cursor = text + ((len + 4) & ~3);
    return 1;
}
#pragma pop

// Create a named BGM play event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundBgmPlayCommand() {
    Fn80097998Arg* self = (Fn80097998Arg*)this;
    // Phase 1: locate the padded name after the 8-byte prefix, skip or wrap the BGM-play event.
    unsigned char* cursor = self->cursor;
    char* text = (char*)cursor + kOneWordCommandBytes;
    if (self->flags) {
        unsigned int len = strlen(text);
        self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
        return 1;
    }
    if (!self->commandTick)
        self->pending = new Fn80097998Node(cursor);
    Item_80097998* item = new Item_80097998(self, cursor);
    if (self->size < self->capacity)
        self->items[self->size++] = (void*)item;
    else {
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_80097998
        VecBuf_80097998 buf(&self->capacity);
        unsigned int minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_80097998
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            unsigned int g = ((cap + 1) * 3) / 5;
            newCap = cap + max_80097998(g, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            unsigned int g = (cap + 1) >> 1;
            newCap = cap + max_80097998(g, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_80097998
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.used = sz;
        buf.first[buf.count + sz] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.used = sz - n;
        memcpy(data + buf.used, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pending = 0;
    unsigned int len = strlen(text);
    self->cursor = (unsigned char*)(text + ((len + 4) & ~3));
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// SC_SND_BGM_STOP: create a stop event and transfer it to the sound callback queue.
// Always asynchronous here. The eight-byte command advances after queueing, not
// after the sound stops. String-copy and vector machinery package/transfer the
// callback; they do not perform the stop themselves. The queue owns the wrapper.
int CEventConteProc::ProcessSoundBgmStopCommand() {
    Fn80097F94Arg* self = (Fn80097F94Arg*)this;
    unsigned int growth1;
    unsigned int growth2;
    unsigned int minGrow;
    register unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kOneWordCommandBytes;
        return 1;
    }
    // Phase 1: create the stop event once, then wrap it with diagnostic names.
    if (!self->commandTick)
        self->pending = new Node_80097F94(cursor);
    Item_80097F94* item = new Item_80097F94(self, cursor);
    if (self->size < self->capacity) {
        unsigned int n = self->size + 1;
        self->size = n;
        self->items[n - 1] = item;
    } else {
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_80097F94
        // Phase 2: guard a larger pointer array; preserve callback order.
        VecBuf_80097F94 buf(&self->capacity);
        minGrow = self->size + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            LE_THROW_80097F94
        unsigned int newCap;
        if (cap < kPointerVectorSmallGrowthLimit) {
            growth1 = ((cap + 1) * 3) / 5;
            newCap = cap + max_80097F94(growth1, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            growth2 = (cap + 1) >> 1;
            newCap = cap + max_80097F94(growth2, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_80097F94
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        unsigned int sz = self->size;
        buf.first = data;
        buf.prefixOffset = sz;
        (buf.first + sz)[buf.count] = (unsigned int)item;
        buf.count = buf.count + 1;
        buf.cap = newCap;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.prefixOffset = sz - n;
        memcpy(data + buf.prefixOffset, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int total = buf.count + n;
        buf.count = total;
        // Commit storage; buf now owns the old array, not any callbacks.
        unsigned int* nf = buf.first;
        unsigned int oc = self->capacity;
        unsigned int nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: wrapper is published; interpreter relinquishes the event pointer.
    self->pending = 0;
    self->cursor = cursor + kOneWordCommandBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Create a BGM volume event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundBgmVolumeCommand() {
    Fn8009856CArg* self = (Fn8009856CArg*)this;
    // Phase 1: skip or create the volume event, then capture it in an owned wrapper.
    unsigned int cap;
    unsigned int size = *(volatile unsigned int*)&self->size;
    unsigned int newCap;
    Item_8009856C* item;
    unsigned int growth1;
    unsigned int growth2;
    unsigned int minGrow;
    register unsigned char* cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kBgmVolumeCommandBytes;
        return 1;
    }
    if (!self->commandTick)
        self->pending = new Fn8009856CNode(cursor);
    item = new Item_8009856C(self, cursor);
    if (size < self->capacity) {
        unsigned int n = size + 1;
        self->size = n;
        self->items[n - 1] = item;
    } else {
        unsigned int oc;
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_8009856C
        VecBuf_80097F94 buf(&self->capacity);
        minGrow = self->size + 1 - self->capacity;
        cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            length_error_8009856C();
        if (cap < kPointerVectorSmallGrowthLimit) {
            growth1 = ((cap + 1) * 3) / 5;
            newCap = cap + max_80097F94(growth1, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            growth2 = (1 + cap) >> 1;
            newCap = cap + max_80097F94(growth2, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_8009856C
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(newCap * 4);
        int sz = self->size;
        buf.first = data;
        buf.prefixOffset = sz;
        buf.cap = newCap;
        (sz + buf.first)[buf.count] = (unsigned int)item;
        buf.count += 1;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.prefixOffset = sz - n;
        memcpy(buf.prefixOffset + data, oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int* nf;
        unsigned int nc;
        unsigned int total;
        total = buf.count + n;
        unsigned int oldCapacity1 = self->capacity;
        buf.count = total;
        nf = buf.first;
        oc = oldCapacity1;
        nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pending = 0;
    self->cursor = cursor + 0xC;
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// SC_SND_BGM_PITCH: create a pitch event and transfer it to the sound callback queue.
int CEventConteProc::ProcessSoundBgmPitchCommand() {
    Fn80098B44Arg* self = (Fn80098B44Arg*)this;
    // Phase 1: skip or create the pitch event, then capture it in an owned wrapper.
    unsigned int growth1;
    unsigned int newCap;
    Item_80098B44* item;
    register unsigned char* cursor = self->cursor;
    unsigned int growth2;
    if (self->flags) {
        self->cursor = cursor + kBgmPitchCommandBytes;
        return 1;
    }
    if (self->commandTickWord == 0)
        self->pendingEvent = new Fn80098B44Node(cursor);
    item = new Item_80098B44(self, cursor);
    if (self->size < self->capacity) {
        unsigned int n = 1 + self->size;
        self->size = n;
        self->items[n - 1] = item;
    } else {
        unsigned int minGrow;
        // Phase 2: reserve a larger pointer buffer, append the new wrapper,
        // transfer old handles in order, then swap storage. The local guard
        // frees the old pointer array, retaining the transferred events.
        if (kMaxPointerVectorSize - self->capacity < 1)
            LE_THROW_80098B44
        VecBuf_80097F94 buf(&self->capacity);
        unsigned int capacityValue = self->capacity;
        unsigned int cap;
        minGrow = self->size + 1 - capacityValue;
        cap = self->capacity;
        if (minGrow > kMaxPointerVectorSize - cap)
            length_error_80098B44();
        if (cap < kPointerVectorSmallGrowthLimit) {
            growth1 = ((cap + 1) * 3) / 5;
            newCap = cap + max_80097F94(growth1, minGrow);
        } else if (cap < kPointerVectorLargeGrowthLimit) {
            growth2 = (cap + 1) >> 1;
            newCap = cap + max_80097F94(growth2, minGrow);
        } else
            newCap = kMaxPointerVectorSize;
        if (newCap > kMaxPointerVectorSize)
            LE_THROW_80098B44
        unsigned int* data = (unsigned int*)appMallocDummy__FUl(4 * newCap);
        buf.first = data;
        unsigned int sz = self->size;
        buf.prefixOffset = sz;
        (buf.first + sz)[buf.count] = (unsigned int)item;
        buf.cap = newCap;
        buf.count = buf.count + 1;
        unsigned int* oldItems = (unsigned int*)self->items;
        unsigned int n = ((unsigned int*)self->items + self->size) - oldItems;
        buf.prefixOffset = sz - n;
        memcpy((data + buf.prefixOffset), oldItems, n * 4);
        memset(oldItems, 0, n * 4);
        unsigned int* nf;
        unsigned int nc;
        int total;
        total = buf.count + n;
        buf.count = total;
        nf = buf.first;
        unsigned int oc;
        oc = self->capacity;
        nc = buf.cap;
        self->size = 0;
        buf.first = (unsigned int*)self->items;
        self->capacity = nc;
        buf.cap = oc;
        self->items = (void**)nf;
        self->size = total;
        buf.count = 0;
    }
    // Phase 3: the queue owns the wrapper/event; clear the interpreter alias.
    // The cursor advances only on the completed or transferred path.
    self->pendingEvent = 0;
    self->cursor = cursor + 0xC;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// SC_SET_FLG: forward the three flag operands to the flag setter.
int CEventConteProc::ProcessSetFlagCommand(Fn8009911CArg* script) {
    unsigned short value = script->flags;
    unsigned char* cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    EventFlag_SetStub(AsEventScriptCommandView(cursor)->operandByte, cursor[2], cursor[3]);
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// SC_SET_VAR_FLG: bind a script variable slot to the selected flag storage.
int CEventConteProc::ProcessBindFlagVariableCommand(Fn80099188Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    void* value = EventFlag_GetStoragePointerStub(cursor[2], cursor[3]);
    unsigned char index = AsEventScriptCommandView(cursor)->operandByte;
    if (index < kScriptSlotCount)
        AsEventScriptStateView(script)->slots[index] = value;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// SC_SET_VAR_PARA: bind a script variable slot to the selected parameter storage.
int CEventConteProc::ProcessBindParameterVariableCommand(Fn80099188Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    void* value = EventParameter_GetValueWord(cursor[2], cursor[3]);
    unsigned char index = AsEventScriptCommandView(cursor)->operandByte;
    if (index < kScriptSlotCount)
        AsEventScriptStateView(script)->slots[index] = value;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// SC_VAR_JMP: conditionally jump to a byte-swapped script offset using a bound variable.
int CEventConteProc::ProcessVariableJumpCommand(Fn80099288Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kTwoWordCommandBytes;
        return 1;
    }
    unsigned short value = (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandWord1);
    int byte = AsEventScriptCommandView(cursor)->operandByte;
    unsigned short swapped = value;
    ((unsigned char*)&swapped)[0] = ((unsigned char*)&value)[1];
    ((unsigned char*)&swapped)[1] = ((unsigned char*)&value)[0];
    void* table = 0;
    table = (byte >= kScriptSlotCount) ? table : AsEventScriptStateView(script)->slots[byte];
    // 0x80099288: slots at +8
    if (Int32_CompareByMode(table, cursor[2], AsEventScriptCommandView(cursor)->operandWord0) == 1)
        script->cursor = script->base + swapped;
    else
        script->cursor = cursor + kTwoWordCommandBytes;
    return 1;
}

// SC_WAIT_CAM: wait until the camera callback queue is empty.
int CEventConteProc::ProcessWaitCameraCommand(Fn80099348Arg* script) {
    unsigned short value = script->flags;
    unsigned int cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->pendingCameraCount != 0) {
        unsigned int count = script->count;
        if (count < kSaturatedCommandTick)
            script->count = count + 1;
        return 0;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma dont_inline on
// SC_WAIT_MDL: wait until both the model queue and keyed model map are empty.
int CEventConteProc::ProcessWaitModelCommand(Fn800993A4Arg* script) {
    unsigned short value = script->flags;
    unsigned int cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->pendingModelCount != 0 || script->pendingKeyedModelCount != 0) {
        unsigned int count = script->count;
        if (count < kSaturatedCommandTick)
            script->count = count + 1;
        return 0;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// SC_WAIT_EFF: wait until the effect callback queue is empty.
int CEventConteProc::ProcessWaitEffectCommand(Fn8009940CArg* script) {
    unsigned short value = script->flags;
    unsigned int cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->pendingEffectCount != 0) {
        unsigned int count = script->count;
        if (count < kSaturatedCommandTick)
            script->count = count + 1;
        return 0;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// SC_WAIT_MSG: wait until the message callback queue is empty.
int CEventConteProc::ProcessWaitMessageCommand(Fn80099468Arg* script) {
    unsigned short value = script->flags;
    unsigned int cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->pendingMessageCount != 0) {
        unsigned int count = script->count;
        if (count < kSaturatedCommandTick)
            script->count = count + 1;
        return 0;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_SET_START_POINT: store the script start point in the owning manager.
int CEventConteProc::ProcessSetStartPointCommand(Fn800994C4Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->object != 0)
        ((ScriptManagerView*)script->object)->startPoint = AsEventScriptCommandView(cursor)->operandShort;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_BLOCK_POINT: request block-point state 5, update its tick count, and wait for completion.
int CEventConteProc::ProcessBlockPointCommand(Fn80099508Arg* script) {
    Fn80099508Entry* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + 1;
        return 1;
    }
    Fn80099508State* blockState = script->object->state;
    if (script->count == 0) {
        blockState->blockTicks = 0;
        blockState->blockPoint = cursor->value;
        blockState->blockState = kBlockPointInProgress;
    }
    if (blockState->blockState == kBlockPointInProgress) {
        blockState->blockTicks = script->count;
        script->count++;
        return 0;
    }
    blockState->blockState = kBlockPointFinished;
    script->cursor = cursor + 1;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// TA_SEAL_POINT: append the point/value pair to the manager seal-point vector.
int CEventConteProc::ProcessSealPointCommand() {
    Fn80099594Arg* self = (Fn80099594Arg*)this;
    // Phase 1: decode the byte point ID and signed halfword value. No callback
    // is constructed: this command appends a record to the manager's vector.
    register unsigned int cap, newCap;
    unsigned int growth1, growth2, minGrow;
    register unsigned char* cursor = (unsigned char*)self->cursor;
    if (self->flags) {
        self->cursor = (Fn80099594Entry*)(cursor + kBasicCommandBytes);
        return 1;
    }
    register Vec_80099594* vec = (Vec_80099594*)self->object;
    register short value = AsEventScriptCommandView(cursor)->operandShort;
    register unsigned int type = AsEventScriptCommandView(cursor)->operandByte;
    if (vec) {
        unsigned int size = *(volatile unsigned int*)&vec->size;
        U4_80099594 z;
        z.c = 0;
        if (size < vec->cap) {
            unsigned int n = size + 1;
            vec->size = n;
            (vec->first + (n - 1))->init(value, type);
        } else {
            U4_80099594 z2;
            z2.c = 0;
            U4_80099594 z3;
            z3.c = 0;
            // Phase 2: grow the 8-byte record vector and copy existing pairs;
            // commit buffer/count/capacity only after the new tail is initialized.
            if (kMaxCommandRecordVectorSize - vec->cap < 1)
                LE_THROW_80099594
            minGrow = vec->size + 1 - vec->cap;
            Buf_80099594 buf(&vec->cap);
            cap = vec->cap;
            if (minGrow > kMaxCommandRecordVectorSize - cap)
                LE_THROW_80099594
            if (cap < kRecordVectorSmallGrowthLimit) {
                growth1 = ((cap + 1) * 3) / 5;
                newCap = cap + max_80097F94(growth1, minGrow);
            } else if (cap < kRecordVectorLargeGrowthLimit) {
                growth2 = (cap + 1) >> 1;
                newCap = cap + max_80097F94(growth2, minGrow);
            } else
                newCap = kMaxCommandRecordVectorSize;
            U4_80099594 z4;
            z4.c = 0;
            if (newCap > kMaxCommandRecordVectorSize)
                LE_THROW_80099594
            Entry_80099594* data = (Entry_80099594*)appMallocDummy__FUl(newCap * 8);
            unsigned int sz = vec->size;
            buf.first = data;
            Entry_80099594* dst = data + sz;
            (dst + buf.count)->init(value, type);
            buf.cap = newCap;
            Entry_80099594* begin = vec->first;
            Entry_80099594* end = begin + vec->size;
            buf.used = sz;
            U4_80099594 z5;
            z5.c = 0;
            buf.count = buf.count + 1;
            U4_80099594 z6;
            z6.c = 0;
            while (end > begin) {
                --dst;
                --end;
                new (dst) Entry_80099594(*end);
                buf.used = buf.used - 1;
                buf.count = buf.count + 1;
            }
            unsigned int total = buf.count;
            vec->size = 0;
            unsigned int nc = buf.cap;
            unsigned int oc = vec->cap;
            Entry_80099594* nf = buf.first;
            vec->cap = nc;
            Entry_80099594* of = vec->first;
            U4_80099594 z7;
            z7.c = 0;
            vec->first = nf;
            buf.cap = oc;
            buf.first = of;
            vec->size = total;
            buf.count = 0;
        }
    }
    self->cursor = (Fn80099594Entry*)(cursor + kBasicCommandBytes);
    return 1;
}
#pragma pop

// TA_MOVE_CHARA: publish the character movement mode and target point to the manager.
int CEventConteProc::ProcessMoveCharacterCommand(Fn80099A88Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    void* object = script->object;
    if (object != 0) {
        ((ScriptManagerView*)object)->characterMoveMode = AsEventScriptCommandView(cursor)->operandByte;
        ((ScriptManagerView*)script->object)->characterMoveTarget = AsEventScriptCommandView(cursor)->operandShort;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// TA_OBJCT_DRAW: append the point/value pair to the manager object-draw vector.
int CEventConteProc::ProcessObjectDrawCommand() {
    Fn80099AD8Arg* self = (Fn80099AD8Arg*)this;
    // Phase 1: decode the byte ID and signed halfword value. No callback is
    // constructed: this command appends a record to the manager's vector at +18.
    unsigned int size, n, minGrow, cap, newCap, growth1, growth2, capacity;
    register short value;
    register unsigned int type;
    register Object_80099AD8* vec;
    register unsigned char* cursor;
    cursor = self->cursor;
    if (self->flags) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    vec = (Object_80099AD8*)self->object;
    value = AsEventScriptCommandView(cursor)->operandShort;
    type = AsEventScriptCommandView(cursor)->operandByte;
    if (vec) {
        size = *(volatile unsigned int*)&vec->size;
        capacity = *(volatile unsigned int*)&vec->cap;
        U4_80099594 z;
        z.c = 0;
        if (size < capacity) {
            n = size + 1;
            vec->size = n;
            (vec->first + (n - 1))->init(value, type);
        } else {
            U4_80099594 z2;
            z2.c = 0;
            U4_80099594 z3;
            z3.c = 0;
            // Phase 2: grow the vector at +18 and transfer its 8-byte records.
            // This is value storage; no callback or event ownership is involved.
            if (kMaxCommandRecordVectorSize - capacity < 1)
                LE_THROW_80099AD8
            minGrow = vec->size + 1 - vec->cap;
            Buf_80099594 buf(&vec->cap);
            cap = vec->cap;
            if (minGrow > kMaxCommandRecordVectorSize - cap)
                LE_THROW_80099AD8
            if (cap < kRecordVectorSmallGrowthLimit) {
                growth1 = ((cap + 1) * 3) / 5;
                newCap = cap + max_80097F94(growth1, minGrow);
            } else if (cap < kRecordVectorLargeGrowthLimit) {
                growth2 = (cap + 1) >> 1;
                newCap = cap + max_80097F94(growth2, minGrow);
            } else
                newCap = kMaxCommandRecordVectorSize;
            U4_80099594 z4;
            z4.c = 0;
            if (newCap > kMaxCommandRecordVectorSize)
                LE_THROW_80099AD8
            Entry_80099594* data = (Entry_80099594*)appMallocDummy__FUl(newCap * 8);
            unsigned int sz = vec->size;
            buf.first = data;
            Entry_80099594* dst = data + sz;
            (dst + buf.count)->init(value, type);
            buf.cap = newCap;
            Entry_80099594* begin = vec->first;
            Entry_80099594* end = begin + vec->size;
            buf.used = sz;
            U4_80099594 z5;
            z5.c = 0;
            buf.count = buf.count + 1;
            U4_80099594 z6;
            z6.c = 0;
            while (end > begin) {
                --dst;
                --end;
                new (dst) Entry_80099594(*end);
                buf.used = buf.used - 1;
                buf.count = buf.count + 1;
            }
            unsigned int total = buf.count;
            vec->size = 0;
            unsigned int nc = buf.cap;
            unsigned int oc = vec->cap;
            Entry_80099594* nf = buf.first;
            vec->cap = nc;
            Entry_80099594* of = vec->first;
            U4_80099594 z7;
            z7.c = 0;
            vec->first = nf;
            buf.cap = oc;
            buf.first = of;
            vec->size = total;
            buf.count = 0;
        }
    }
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// Request hint-window loading, wait for readiness, then select its signed mode operand.
int ScriptCommand_WaitForTourHintState2(Fn80099FCCArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->count == 0) {
        if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
            if (s_tourHintWindowInitialized == 0) {
                ((CTourHintWindow*)s_tourHintWindow)->__ct();
                __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
                s_tourHintWindowInitialized = 1;
            }
            ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
        }
        ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->SetState(kWindowLoadRequested);
        script->count++;
    }
    if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    int result = ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->IsState(kWindowReady);
    if (result == 0) {
        return 0;
    }
    if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))
        ->SelectPage((signed char)AsEventScriptCommandView(cursor)->operandByte);
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_DELETE_HINT_WIN: clear the tour hint window registered objects.
int ScriptCommand_ClearTourHintObjects(Fn8009A170Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->ClearRegisteredObjects();
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma opt_propagation off
// TA_SET_WIN_FADE: initialize the main hint fade and wait while updating its opacity.
int Script_FadeTourHintWindow(Fn8009A22CArg* self) {
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    // Phase 1: first-entry setup; initialization blocks below only obtain the window.
    if (self->count == 0) {
        unsigned char mode = AsEventScriptCommandView(cursor)->operandByte;
        if (mode == kFadeInCommand) {
            EnsureHintWindowInstance();
            ((CFade*)((char*)((unsigned int&)Singleton_TourHintWindow()) +
                      SCRIPT_MEMBER_OFFSET(TourHintFadeView, fadeControl)))
                ->StartFadeIn();
        } else {
            EnsureHintWindowInstance();
            ((CFade*)((char*)((unsigned int&)Singleton_TourHintWindow()) +
                      SCRIPT_MEMBER_OFFSET(TourHintFadeView, fadeControl)))
                ->StartFadeOut();
        }
        EnsureHintWindowInstance();
        char* singletonPtr = (char*)((unsigned int&)Singleton_TourHintWindow());
        unsigned short durationTicks = (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandShort);
        int opacityStep;
        if (durationTicks != 0)
            opacityStep = kFullOpacity / durationTicks;
        else
            opacityStep = kFullOpacity;
        ((CFade*)(singletonPtr + SCRIPT_MEMBER_OFFSET(TourHintFadeView, fadeControl)))
            ->SetStep((unsigned char)opacityStep);
    }
    // Phase 2: publish opacity to the child UI, then tick the fade to completion.
    unsigned int commandTick = self->count;
    unsigned int windowInstance = ((unsigned int&)Singleton_TourHintWindow());
    commandTick = commandTick + 1;
    self->count = commandTick;
    if (windowInstance == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    unsigned char opacity = ((TourHintFadeView*)((unsigned int&)Singleton_TourHintWindow()))->opacity;
    EnsureHintWindowInstance();
    ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->SetNavigationOpacity(opacity);
    EnsureHintWindowInstance();
    ((CTourMessage*)(((unsigned int&)Singleton_TourHintWindow())))->ApplyGlyphColorMask(opacity);
    EnsureHintWindowInstance();
    if (!((CFade*)((char*)((unsigned int&)Singleton_TourHintWindow()) +
                   SCRIPT_MEMBER_OFFSET(TourHintFadeView, fadeControl)))
             ->Update(kFloat_1_TourWindow))
        return 0;
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// TA_SET_SUBWIN_FADE: initialize the hint message fade and wait for completion.
int EventScript_ProcessTourHintFade(Fn8009A570Arg* self) {
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (self->count == 0) {
        if (AsEventScriptCommandView(cursor)->operandByte == kFadeInCommand) {
            if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
                if (s_tourHintWindowInitialized == 0) {
                    ((CTourHintWindow*)s_tourHintWindow)->__ct();
                    __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
                    s_tourHintWindowInitialized = 1;
                }
                ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
            }
            ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->StartFadeIn();
        } else {
            if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
                if (s_tourHintWindowInitialized == 0) {
                    ((CTourHintWindow*)s_tourHintWindow)->__ct();
                    __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
                    s_tourHintWindowInitialized = 1;
                }
                ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
            }
            ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->StartFadeOut();
        }
        if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
            if (s_tourHintWindowInitialized == 0) {
                ((CTourHintWindow*)s_tourHintWindow)->__ct();
                __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
                s_tourHintWindowInitialized = 1;
            }
            ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
        }
        unsigned short durationTicks = (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandShort);
        ((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))
            ->SetFadeStep((unsigned char)(durationTicks ? kFullOpacity / durationTicks : kFullOpacity));
    }
    unsigned int commandTick = self->count + 1;
    unsigned int windowInstance = ((unsigned int&)Singleton_TourHintWindow());
    self->count = commandTick;
    if (windowInstance == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    if (!((CTourHintWindow*)(((unsigned int&)Singleton_TourHintWindow())))->UpdateFade(kFloat_1_TourWindow))
        return 0;
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Expand a hint message id, process its input over ticks, and release its draw object.
int EventScript_ProcessTourHintMessage(Fn8009A790Arg* self) {
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (self->index == 0) {
        WString result;
        result.flags = 0;
        result.length = 0;
        result.data = 0;
        if (((unsigned int&)Sing_805DC670()) == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
                s_bpPlayerInitialized = 1;
            }
            ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
        }
        void* data = StoryMessageScalar_GetValue((void*)((unsigned int&)Sing_805DC670()));
        MessageTable_ExpandEntryText(data, &result, (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandShort),
                                     0);
        if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
            if (s_tourHintWindowInitialized == 0) {
                ((CTourHintWindow*)s_tourHintWindow)->__ct();
                __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
                s_tourHintWindowInitialized = 1;
            }
            ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
        }
        TourHintMessage_SetText((void*)((unsigned int&)Singleton_TourHintWindow()), &result);
    }
    if (((unsigned int&)Singleton_TourHintWindow()) == 0) {
        if (s_tourHintWindowInitialized == 0) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, s_tourHintWindowDtorRegistration);
            s_tourHintWindowInitialized = 1;
        }
        ((unsigned int&)Singleton_TourHintWindow()) = (unsigned int)s_tourHintWindow;
    }
    if (!((CTourMessage*)(((unsigned int&)Singleton_TourHintWindow())))->ProcessInput(self->index)) {
        if (self->index < kSaturatedCommandTick)
            ++self->index;
        return 0;
    }
    if (((unsigned int&)Sing_805DC670()) == 0) {
        if (s_bpPlayerInitialized == 0) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
    }
    TourMessageOwner_DestroyMessageDraw((void*)((unsigned int&)Sing_805DC670()));
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Request versus-window loading, wait for readiness, then initialize the requested layout.
int EventCommand_InitTourVSWindow(Fn8009A9C4Arg* script) {
    unsigned short value = script->flags;
    unsigned char* cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->count == 0) {
        if (((unsigned int&)Sing_805DC690()) == 0) {
            if (s_tourVsWindowInitialized == 0) {
                ((CTourVSWindow*)s_tourVSWindow)->__ct();
                __register_global_object(s_tourVSWindow, (void (*)())Destroy__13CTourVSWindowFi,
                                         s_tourVsWindowDestructorRegistration);
                s_tourVsWindowInitialized = 1;
            }
            ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
        }
        TourVSWindow_SetStatus((void*)((unsigned int&)Sing_805DC690()), kWindowLoadRequested);
        unsigned int c = script->count;
        script->count = c + 1;
    }
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, (void (*)())Destroy__13CTourVSWindowFi,
                                     s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    if (TourVSWindow_HasStatus((void*)((unsigned int&)Sing_805DC690()), kWindowReady) == 0)
        return 0;
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, (void (*)())Destroy__13CTourVSWindowFi,
                                     s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    ((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))->InitLayout(AsEventScriptCommandView(cursor)->operandShort);
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Fade out the versus window, wait for completion, then reset and hide it.
int EventCommand_FadeOutTourVSWindow(Fn8009AB64Arg* script) {
    unsigned int cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (script->count == 0) {
        if (((unsigned int&)Sing_805DC690()) == 0) {
            if (s_tourVsWindowInitialized == 0) {
                ((CTourVSWindow*)s_tourVSWindow)->__ct();
                __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi,
                                         s_tourVsWindowDestructorRegistration);
                s_tourVsWindowInitialized = 1;
            }
            ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
        }
        ((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
            ->StartFadeOut();
        if (((unsigned int&)Sing_805DC690()) == 0) {
            if (s_tourVsWindowInitialized == 0) {
                ((CTourVSWindow*)s_tourVSWindow)->__ct();
                __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi,
                                         s_tourVsWindowDestructorRegistration);
                s_tourVsWindowInitialized = 1;
            }
            ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
        }
        ((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
            ->SetStep(kTourVSFadeOutStep);
        if (((unsigned int&)Sing_805DC690()) == 0) {
            if (s_tourVsWindowInitialized == 0) {
                ((CTourVSWindow*)s_tourVSWindow)->__ct();
                __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi,
                                         s_tourVsWindowDestructorRegistration);
                s_tourVsWindowInitialized = 1;
            }
            ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
        }
        TourVSWindow_StartFadeOut((void*)((unsigned int&)Sing_805DC690()));
    }
    unsigned int count = ((volatile Fn8009AB64Arg*)script)->count;
    script->count = count + 1;
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    if (((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
            ->Update(kFloat_1_TourWindow) == 0)
        return 0;
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    ((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))->ResetAndHide();
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma opt_propagation off
// Start the versus-window transition and wait for both its fade and transition to finish.
int Script_StartTourVSWindowTransition(Fn8009ADBCArg* self) {
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (self->count == 0) {
        EnsureVSWindowInstance();
        ((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))->BeginTransition();
        EnsureVSWindowInstance();
        ((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
            ->StartFadeIn();
        EnsureVSWindowInstance();
        ((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
            ->SetStep(kFullOpacity);
    }
    unsigned int commandTick = self->count;
    unsigned int windowInstance = ((unsigned int&)Sing_805DC690());
    ++commandTick;
    self->count = commandTick;
    if (windowInstance == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    unsigned char opacity = ((TourVSWindowData*)((unsigned int&)Sing_805DC690()))->opacity;
    EnsureVSWindowInstance();
    ((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))->SetGroupOpacity((void*)opacity);
    EnsureVSWindowInstance();
    if (!((CFade*)((char*)((unsigned int&)Sing_805DC690()) + SCRIPT_MEMBER_OFFSET(TourVSWindowData, fadeControl)))
             ->Update(kFloat_1_TourWindow))
        return 0;
    EnsureVSWindowInstance();
    if (!((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))->UpdateTransition(kFloat_1_TourWindow))
        return 0;
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// Expand a versus message id, process its input over ticks, and release its draw object.
int EventScript_ProcessTourVersusMessage(Fn8009B0E0Arg* self) {
    unsigned char* cursor = self->cursor;
    if (self->flags != 0) {
        self->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    if (self->index == 0) {
        WString result;
        result.flags = 0;
        result.length = 0;
        result.data = 0;
        if (((unsigned int&)Sing_805DC670()) == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
                s_bpPlayerInitialized = 1;
            }
            ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
        }
        void* data = StoryMessageScalar_GetValue((void*)((unsigned int&)Sing_805DC670()));
        MessageTable_ExpandEntryText(data, &result, (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandShort),
                                     0);
        if (((unsigned int&)Sing_805DC690()) == 0) {
            if (s_tourVsWindowInitialized == 0) {
                ((CTourVSWindow*)s_tourVSWindow)->__ct();
                __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi,
                                         s_tourVsWindowDestructorRegistration);
                s_tourVsWindowInitialized = 1;
            }
            ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
        }
        ((CTourVSWindow*)(((unsigned int&)Sing_805DC690())))
            ->SetMessageMode(AsEventScriptCommandView(cursor)->operandByte, (TourMessageText*)&result);
    }
    if (((unsigned int&)Sing_805DC690()) == 0) {
        if (s_tourVsWindowInitialized == 0) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, s_tourVsWindowDestructorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((unsigned int&)Sing_805DC690()) = (unsigned int)s_tourVSWindow;
    }
    if (!((CTourMessage*)(((unsigned int&)Sing_805DC690())))->ProcessInput(self->index)) {
        if (self->index < kSaturatedCommandTick)
            ++self->index;
        return 0;
    }
    if (((unsigned int&)Sing_805DC670()) == 0) {
        if (s_bpPlayerInitialized == 0) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_bpPlayerDtorRegistration_Exhibition);
            s_bpPlayerInitialized = 1;
        }
        ((unsigned int&)Sing_805DC670()) = (unsigned int)s_bpPlayer;
    }
    TourMessageOwner_DestroyMessageDraw((void*)((unsigned int&)Sing_805DC670()));
    self->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_EVENT_SKIP: clear the manager event-skip field and consume the command.
int CEventConteProc::ProcessEventSkipCommand(ScriptCommandContextView* script) {
    unsigned short value = script->flags;
    void* object;
    unsigned char* cursor = script->cursor;
    if (value != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    object = script->manager;
    if (object != 0)
        ((ScriptManagerView*)object)->eventSkip = 0;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Show a yes/no dialog; wait for a result and count Yes as a true condition.
int CEventConteProc::ProcessIsYesNoDialogCommand(ScriptCommandContextView* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags & kOuterConditionMask) {
        script->cursor = (cursor + kBasicCommandBytes);
        return 1;
    }
    if (script->elapsedCommandTicks == 0) {
        if (*((Fn8009B35CObject*)script->manager)->dialogSlot) {
            if (*((Fn8009B35CObject*)script->manager)->dialogSlot) {
                void** vt = *(void***)*((Fn8009B35CObject*)script->manager)->dialogSlot;
                ((void (*)(void*, int))vt[2])(*((Fn8009B35CObject*)script->manager)->dialogSlot, 1);
                *((Fn8009B35CObject*)script->manager)->dialogSlot = 0;
            }
        }
        *((Fn8009B35CObject*)script->manager)->dialogSlot = new Fn8009B35CNode(kYesNoDialogKind, kDialogInputDelay);
        ((CPangyaCommonDialog*)(*((Fn8009B35CObject*)script->manager)->dialogSlot))
            ->SetPosition(kDialogLeft, kDialogTop);
        ((CPangyaCommonDialog*)(*((Fn8009B35CObject*)script->manager)->dialogSlot))->SetWindowEnabled(0);
        ((CFlashMemoryMenu*)(*((Fn8009B35CObject*)script->manager)->dialogSlot))->SetSavingIndicatorEnabled(0);
        ((CPangyaCommonDialog*)(*((Fn8009B35CObject*)script->manager)->dialogSlot))
            ->SetText(s_eventScriptEmptyDialogText);
    }
    ++script->elapsedCommandTicks;
    void* item = *((Fn8009B35CObject*)script->manager)->dialogSlot;
    int state = ((CPangyaCommonDialog*)item)->result;
    if (state == kDialogYes) {
        if (item) {
            if (item) {
                void** vt = *(void***)item;
                ((void (*)(void*, int))vt[2])(item, 1);
            }
        }
        *((Fn8009B35CObject*)script->manager)->dialogSlot = 0;
        script->cursor = (cursor + kBasicCommandBytes);
        script->conditionCount = (1 + script->conditionCount);
        script->trueConditionCount += 1;
        return 1;
    }
    if (state == kDialogNo) {
        if (item) {
            if (item) {
                void** vt = *(void***)item;
                ((void (*)(void*, int))vt[2])(item, 1);
            }
        }
        *((Fn8009B35CObject*)script->manager)->dialogSlot = 0;
        script->conditionCount = (1 + script->conditionCount);
        script->cursor = (cursor + kBasicCommandBytes);
        return 1;
    }
    return 0;
}

// SC_IS_TOUR_FLAG: test a selected tour-point or record flag and accumulate the condition.
int CEventConteProc::ProcessIsTourFlagCommand(Fn8009BTargetArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags & kOuterConditionMask) {
        script->cursor = (kBasicCommandBytes + cursor);
        return 1;
    }
    Fn8009BTree* tree = (Fn8009BTree*)script->object;
    unsigned char* manager = (unsigned char*)*tree->node;
    unsigned char* base = manager + SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress);
    int lane = ((ScriptSaveProgressView*)manager)->selectedLane;
    unsigned char* table =
        base + ((signed char*)base)[lane + SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, selectedRowByLane) -
                                    SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress)] *
                   sizeof(ScriptProgressRow);
    int matched = 0;
    int index = ((ScriptProgressRow*)table)->selectedEntry;
    switch (AsEventScriptCommandView(cursor)->operandByte) {
    case 0:
        matched = (((ScriptProgressRow*)table)->entryFlags[index] >> 12) & 1;
        break;
    case 1:
        matched = (((ScriptProgressRow*)table)->entryFlags[index] >> 11) & 1;
        break;
    case 2:
        matched = (base[SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, tourFlags) -
                        SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress)] >>
                   6) &
                  1;
        break;
    case 3:
        matched = (base[0x33A] >> 7) & 1;
        break;
    case 4:
        matched = (((ScriptProgressRow*)table)->entryFlags[index] >> 4) & 1;
        break;
    }
    script->conditionCount = (1 + script->conditionCount);
    if (matched)
        script->trueConditionCount++;
    script->cursor = (cursor + kBasicCommandBytes);
    return 1;
}

// SC_IS_CHARA: count a true condition when the equipped character matches the operand.
int CEventConteProc::ProcessIsCharacterCommand(Fn8009B654Arg* script) {
    unsigned short value = script->flags;
    unsigned char* cursor = script->cursor;
    if (value & kOuterConditionMask) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    int matched = 0;
    if (AsEventScriptCommandView(cursor)->operandByte == EventScript_GetCharacterIndex())
        matched = 1;
    script->conditionCount++;
    if (matched)
        script->trueConditionCount++;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// SC_IS_CADDIE: count a true condition when the equipped caddie matches the operand.
int CEventConteProc::ProcessIsCaddieCommand(Fn8009BTargetArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags & kOuterConditionMask) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    int matched = 0;
    if (AsEventScriptCommandView(cursor)->operandByte == EventScript_GetCaddieIndex())
        matched = 1;
    script->conditionCount++;
    if (matched)
        script->trueConditionCount++;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// SC_IS_COSTUME: evaluate the requested costume condition and accumulate its result.
int CEventConteProc::ProcessIsCostumeCommand(Fn8009B794Arg* script) {
    unsigned char* cursor = script->cursor;
    if ((script->flags & kOuterConditionMask) != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    int result = EventScript_IsCostumePartEquipped(AsEventScriptCommandView(cursor)->operandShort);
    unsigned char count = script->conditionCount + 1;
    script->conditionCount = count;
    if (result != 0) {
        unsigned char success = script->trueConditionCount + 1;
        script->trueConditionCount = success;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// SC_IS_CLUBSET: evaluate the requested club-set condition and accumulate its result.
int CEventConteProc::ProcessIsClubSetCommand(Fn8009B794Arg* script) {
    unsigned char* cursor = script->cursor;
    if ((script->flags & kOuterConditionMask) != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    int result = EventScript_IsClubSetEquipped((signed char)AsEventScriptCommandView(cursor)->operandByte);
    unsigned char count = script->conditionCount + 1;
    script->conditionCount = count;
    if (result != 0) {
        unsigned char success = script->trueConditionCount + 1;
        script->trueConditionCount = success;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// Count a true condition when the saved coin count reaches the unsigned threshold.
int CEventConteProc::ProcessIsCoinNumCommand(Fn8009B8A0Arg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags & kOuterConditionMask) {
        unsigned char* sum = cursor + kBasicCommandBytes;
        script->cursor = sum;
        return 1;
    }
    if (0 == ((unsigned int&)GetSaveDataSlot())) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
    }
    ScriptSaveProgressView* manager =
        (ScriptSaveProgressView*)SaveData_GetCurrentRecord((void*)((unsigned int&)GetSaveDataSlot()));
    unsigned int threshold = (*(unsigned short*)&AsEventScriptCommandView(cursor)->operandShort);
    unsigned int boundary = manager->coinCount;
    // Unsigned boundary >= threshold, expressed in the original branch-generating form.
    unsigned int delta = boundary - threshold;
    unsigned int limitBase = boundary | ~threshold;
    unsigned char count0 = script->conditionCount;
    script->conditionCount = 1 + count0;
    int limit = limitBase - (delta >> 1);
    if ((int)limit < 0)
        script->trueConditionCount++;
    script->cursor = kBasicCommandBytes + cursor;
    return 1;
}

// SC_IS_BLOCK: compare the requested point block flag with the command boolean.
int CEventConteProc::ProcessIsBlockCommand(Fn8009BTargetArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags & kOuterConditionMask) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    Fn8009BTree* tree = (Fn8009BTree*)script->object;
    unsigned char* manager = (unsigned char*)*tree->node;
    int index = AsEventScriptCommandView(cursor)->operandShort;
    unsigned char* base = manager + SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress);
    int lane = ((ScriptSaveProgressView*)manager)->selectedLane;
    unsigned char* entry = base + lane;
    int subindex = ((signed char*)entry)[SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, selectedRowByLane) -
                                         SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress)];
    entry = base + subindex * sizeof(ScriptProgressRow) + index * 2;
    int matched =
        AsEventScriptCommandView(cursor)->operandByte == ((*(unsigned short*)(entry + 4) >> kPointBlockedBit) & 1);
    script->conditionCount++;
    if (matched)
        script->trueConditionCount++;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_SET_APPEAR_ROAD_INFO: request a road reveal when its variant flag is still unset.
int CEventConteProc::ProcessSetAppearRoadInfoCommand(Fn8009BTargetArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    // Packed selector: quotient by 100 selects the road variant (zero/nonzero);
    // remainder selects the point in the selected progress row, without bounds checks.
    int roadPointSelector = AsEventScriptCommandView(cursor)->operandShort;
    int hundred = roadPointSelector / 100;
    int remainder = roadPointSelector - hundred * 100;
    Fn8009BTree* tree = (Fn8009BTree*)script->object;
    unsigned char* manager = (unsigned char*)*tree->node;
    unsigned char* base = manager + SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress);
    int lane = ((ScriptSaveProgressView*)manager)->selectedLane;
    int sub = ((signed char*)base)[lane + SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, selectedRowByLane) -
                                   SCRIPT_MEMBER_OFFSET(ScriptSaveProgressView, progress)];
    unsigned short* entry = (unsigned short*)(base + sub * sizeof(ScriptProgressRow) + remainder * 2 + 4);
    int bad = 0;
    if (hundred != 0 && ((*entry >> kRoadHundredsVariantSeenBit) & 1) == 0)
        bad = 1;
    if (hundred == 0 && ((*entry >> kRoadBaseVariantSeenBit) & 1) == 0)
        bad = 1;
    if (bad) {
        unsigned char* state = (unsigned char*)tree->node;
        ((ScriptTourStateView*)state)->requestState = 1;
        ((ScriptTourStateView*)state)->requestedEntry = roadPointSelector;
        ((ScriptTourStateView*)state)->requestComplete = 0;
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_APPEAR_ROAD: enter road-reveal state 3 from state 2 and wait until it changes.
int CEventConteProc::ProcessAppearRoadCommand(Fn8009BTargetArg* script) {
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    Fn8009BState* state = ((Fn8009BStateRoot*)script->object)->state;
    if (script->count == 0) {
        state->elapsedTicks = 0;
        if (state->requestState != kRoadReadyToReveal) {
            state->requestState = kRoadInactive;
            script->cursor = cursor + kBasicCommandBytes;
            return 1;
        }
        state->requestState = kRoadRevealInProgress;
    }
    if (state->requestState == kRoadRevealInProgress) {
        state->elapsedTicks = script->count;
        script->count++;
        return 0;
    }
    state->requestState = kRoadInactive;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

#pragma push
#pragma dont_inline on
// Count a true condition when the current saved tour has its clear bit set.
int CEventConteProc::ProcessIsTourClearCommand(void* script) {
    unsigned short flags = AsEventScriptStateView(script)->flags;
    unsigned int cursor = (*(unsigned int*)&AsEventScriptStateView(script)->cursor);
    if ((flags & kOuterConditionMask) != 0) {
        (*(unsigned int*)&AsEventScriptStateView(script)->cursor) = cursor + kBasicCommandBytes;
        return 1;
    }
    void* manager = AsEventScriptStateView(script)->manager;
    unsigned char* list = (unsigned char*)((ScriptManagerView*)manager)->tourState;
    void* entry = *(void**)list;
    unsigned char flag = ((ScriptSaveProgressView*)entry)->tourFlags;
    AsEventScriptStateView(script)->conditionCount =
        (unsigned char)(AsEventScriptStateView(script)->conditionCount + 1);
    if ((flag >> kTourClearedBit) & 1) {
        unsigned char count7 = AsEventScriptStateView(script)->trueConditionCount;
        AsEventScriptStateView(script)->trueConditionCount = count7 + 1;
    }
    (*(unsigned int*)&AsEventScriptStateView(script)->cursor) = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// SC_SND_VS_SE_PLAY: build and play the selected character voice with the _vs suffix.
int EventCommand_PlayTourVoiceVariant5(ScriptCommandContextView* script) {
    // Phase 1: select the saved character and create the owned base voice name.
    struct StringCleanup {
        Fn8009BBFCString* string;
        // Release the local string buffer if it uses owned heap storage.
        ~StringCleanup() {
            if (string && (AsMWStringStorage(string)->flags >> 31))
                appFreeUni__FPv(AsMWStringStorage(string)->data);
        }
    };
    unsigned char* cursor = script->cursor;
    if (script->flags != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    unsigned int len1;
    unsigned char index =
        *(unsigned char*)&AsSavePlayerRecord(*(void**)((ScriptManagerView*)script->manager)->tourState)->characterIndex;
    Fn8009BBFCString third;
    Fn8009BBFCString second;
    Fn8009BBFCString first;
    Fn8009DB08Tag copy3;
    Fn8009DB08Tag tag3;
    Fn8009DB08Tag copy2;
    Fn8009DB08Tag tag2;
    Fn8009DB08Tag copy1;
    Fn8009DB08Tag tag1;
    AsMWStringStorage(&first)->flags = 0;
    AsMWStringStorage(&first)->length = 0;
    *(unsigned int*)&first.data[8] = 0;
    len1 = first.data[0] & 0x7F;
    copy1.value = tag1.value;
    StringCleanup firstCleanup = {&first};
    ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&first, len1, 0, tourVoiceResourceName,
                                                        tourVoiceResourceName + strlen(tourVoiceResourceName), &copy1);
    Character_GetAssetNameSuffix(&second, (signed char)index);
    // Phase 2: append the character suffix, then the event-5 (_vs) suffix.
    StringCleanup secondCleanup = {&second};
    {
        unsigned int len;
        const char* data;
        unsigned int size;
        if ((AsMWStringStorage(&first)->flags >> 31) == 0)
            len = first.data[0] & 0x7F;
        else
            len = AsMWStringStorage(&first)->length;
        if ((AsMWStringStorage(&second)->flags >> 31) == 0) {
            data = (const char*)&second.data[1];
            size = second.data[0] & 0x7F;
        } else {
            data = AsMWStringStorage(&second)->data;
            size = AsMWStringStorage(&second)->length;
        }
        copy2.value = tag2.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&first, len, 0, data, data + size, &copy2);
    }
    if (AsMWStringStorage(&second)->flags >> 31)
        appFreeUni__FPv(AsMWStringStorage(&second)->data);
    secondCleanup.string = 0;
    Event_GetAssetNameSuffix(&third, 5);
    StringCleanup thirdCleanup = {&third};
    {
        unsigned int len;
        const char* data;
        unsigned int size;
        if ((AsMWStringStorage(&first)->flags >> 31) == 0)
            len = first.data[0] & 0x7F;
        else
            len = AsMWStringStorage(&first)->length;
        if ((AsMWStringStorage(&third)->flags >> 31) == 0) {
            data = (const char*)&third.data[1];
            size = third.data[0] & 0x7F;
        } else {
            data = AsMWStringStorage(&third)->data;
            size = AsMWStringStorage(&third)->length;
        }
        copy3.value = tag3.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&first, len, 0, data, data + size, &copy3);
    }
    if (AsMWStringStorage(&third)->flags >> 31)
        appFreeUni__FPv(AsMWStringStorage(&third)->data);
    thirdCleanup.string = 0;
    {
        const char* data;
        if ((AsMWStringStorage(&first)->flags >> 31) == 0)
            data = (const char*)&first.data[1];
        else
            data = AsMWStringStorage(&first)->data;
        if (AsMWStringStorage(&first)->flags >> 31)
            appFreeUni__FPv(AsMWStringStorage(&first)->data);
        firstCleanup.string = 0;
        // Phase 3: play the name. Existing lifetime caveat: the heap branch above
        // frees data BEFORE this call; this code preserves that order.
        Sound_PlayByName((void*)g_soundManager, data, 0, 0, 0, 0, kFloat_0_5_EventScriptSoundMinimumGain,
                         kFloat_200_EventScriptSoundRange, kFloat_1_TourWindow);
    }
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}

// TA_CONTE_SKIP: set the tour-state skip/exit request and advance the script.
int CEventConteProc::ProcessConteSkipCommand(Fn8009BDFCArg* script) {
    unsigned int* cursor = (unsigned int*)script->cursor;
    if (script->flags != 0) {
        script->cursor = (unsigned int)(cursor + 1);
        return 1;
    }
    ((ScriptTourStateView*)((ScriptManagerView*)script->object)->tourState)->exitRequested = 1;
    script->cursor = (unsigned int)(cursor + 1);
    return 1;
}

#pragma push
#pragma dont_inline on
// Count a true condition when the party result tier matches the signed operand.
int ScriptCondition_CheckPartyResultTier(Fn8009BE3CArg* script) {
    unsigned short value = script->flags;
    unsigned char* cursor = script->cursor;
    if ((value & kOuterConditionMask) != 0) {
        script->cursor = cursor + kBasicCommandBytes;
        return 1;
    }
    signed char target = (signed char)AsEventScriptCommandView(cursor)->operandByte;
    int current = (int)g_sharedDoc->partyResultTier;
    int diff = current - target;
    int isZero = (diff == 0);
    unsigned char counter = script->conditionCount + 1;
    script->conditionCount = counter;
    if (isZero)
        script->trueConditionCount += 1;
    script->cursor = cursor + kBasicCommandBytes;
    return 1;
}
#pragma pop

// Read condition flags, optionally excluding the enclosing condition bit.
unsigned short EventScript_GetConditionFlags(Fn8009BEA4Arg* script, int ignoreOuterCondition) {
    unsigned short flags = script->flags;
    if (ignoreOuterCondition != 0)
        flags &= kOuterConditionMask;
    return flags;
}

// Read one indexed word and reverse its byte order for this operand encoding.
unsigned int WordArray_LoadByteSwapped(unsigned int* words, unsigned char index) {
    union {
        unsigned int word;
        unsigned char bytes[4];
    } value;
    value.word = words[index];
    unsigned char fourth = value.bytes[3];
    unsigned char first = value.bytes[0];
    unsigned char cachedByte1 = value.bytes[1];
    unsigned char cachedByte2 = cachedByte1;
    unsigned char capturedValue = value.bytes[1];
    unsigned char firstValue = first;
    unsigned char fourthValue = fourth;
    value.bytes[0] = fourthValue;
    value.bytes[3] = firstValue;
    value.bytes[1] = (value.bytes[2]);
    unsigned char second = capturedValue;
    unsigned char cachedByte3 = second;
    unsigned char cachedByte4 = cachedByte3;
    value.bytes[2] = cachedByte4;
    return value.word;
}

#pragma push
#pragma use_lmw_stmw on
// Initialize an interpreter with its name, script base, entry cursor, queues, and empty object maps.
CEventHandle::CEventHandle(const char* text, void* scriptData, unsigned int entryAddress) {
    ScriptInterpreterStorage* self = (ScriptInterpreterStorage*)this;
    // Phase 1: establish empty string, queues, map sentinels and effect subobject.
    self->vtable = __vt__12CEventHandle;
    self->first.flags = 0;
    self->first.size = 0;
    self->first.heapAddressOrInlineTail = 0;
    self->vectors[kCameraQueueIndex].data = 0;
    self->vectors[kCameraQueueIndex].size = 0;
    self->vectors[kCameraQueueIndex].capacity = 0;
    self->vectors[kModelQueueIndex].data = 0;
    self->vectors[kModelQueueIndex].size = 0;
    self->vectors[kModelQueueIndex].capacity = 0;
    self->vectors[kEffectQueueIndex].data = 0;
    self->vectors[kEffectQueueIndex].size = 0;
    self->vectors[kEffectQueueIndex].capacity = 0;
    self->vectors[kMessageQueueIndex].data = 0;
    self->vectors[kMessageQueueIndex].size = 0;
    self->vectors[kMessageQueueIndex].capacity = 0;
    self->vectors[kSoundQueueIndex].data = 0;
    self->vectors[kSoundQueueIndex].size = 0;
    self->vectors[kSoundQueueIndex].capacity = 0;
    self->tree0.size = 0;
    self->tree0.root = 0;
    self->tree0.begin = &self->tree0.root;
    ((CEventEffect*)(self->sub))->Init();
    self->tree1.size = 0;
    self->tree1.root = 0;
    self->tree1.begin = &self->tree1.root;
    // Phase 2: deep-copy the interpreter prefix, then install borrowed script
    // addresses and clear the conditions, slots and transient command state.
    union {
        ScriptTreeLinks* root;
        unsigned int oldStringLength;
    } temp;
    temp.oldStringLength =
        ((self->first.flags >> 31) == 0) ? *(unsigned char*)&self->first.flags & 0x7f : self->first.size;
    union {
        volatile unsigned char c;
        int align;
    } source, end;
    end.c = source.c;
    ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(&self->first, 0, temp.oldStringLength, (const char*)text,
                                                        (const char*)text + strlen((const char*)text), &end);
    self->user0 = scriptData;
    self->user1 = (void*)entryAddress;
    memset(self, 0, 8);
    memset((char*)self + 8, 0, 0x400);
    unsigned int n0, n1, n2, n3, n4;
    n0 = self->vectors[0].size;
    n1 = self->vectors[1].size;
    n2 = self->vectors[2].size;
    n3 = self->vectors[3].size;
    n4 = self->vectors[4].size;
    self->state[kCommandTicksWord] = 0;
    self->state[kOwnedHandler0Word] = 0;
    self->state[kPendingModelWord] = 0;
    self->state[kPendingEffectWord] = 0;
    self->state[kPendingMessageWord] = 0;
    self->state[kPendingSoundWord] = 0;
    self->vectors[0].size -= ((char*)self->vectors[0].data + n0) - (char*)self->vectors[0].data;
    self->vectors[1].size -= ((char*)self->vectors[1].data + n1) - (char*)self->vectors[1].data;
    self->vectors[2].size -= ((char*)self->vectors[2].data + n2) - (char*)self->vectors[2].data;
    self->vectors[3].size -= ((char*)self->vectors[3].data + n3) - (char*)self->vectors[3].data;
    self->vectors[4].size -= ((char*)self->vectors[4].data + n4) - (char*)self->vectors[4].data;
    // Phase 3: reset-style keyed-model tree cleanup. Phase 1 made this tree
    // empty in normal construction; retain the unrolled frees and sentinel reset.
    temp.root = self->tree0.root;
    if (temp.root) {
        ScriptTreeLinks* cachedLeft1 = temp.root->left;
        ScriptTreeLinks* a = cachedLeft1;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    ScriptTreeLinks* cValue = c;
                    appFreeUni__FPv(cValue);
                }
                appFreeUni__FPv(b);
            }
            b = a->right;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    ScriptTreeLinks* cValue2 = c;
                    appFreeUni__FPv(cValue2);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        a = temp.root->right;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* cachedLeft2 = b->left;
                ScriptTreeLinks* c = cachedLeft2;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    ScriptTreeLinks* cValue3 = c;
                    appFreeUni__FPv(cValue3);
                }
                appFreeUni__FPv(b);
            }
            b = a->right;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    ScriptTreeLinks* cValue4 = c;
                    appFreeUni__FPv(cValue4);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree0, c->right);
                    ScriptTreeLinks* cValue5 = c;
                    appFreeUni__FPv(cValue5);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        appFreeUni__FPv(temp.root);
        self->tree0.size = 0;
        self->tree0.root = 0;
        self->tree0.begin = &self->tree0.root;
    }
    // Phase 4: reset the face map and +48C face alias, then null the two tail
    // fields. The same empty-map invariant applies to this cleanup expansion.
    ScriptTreeLinks* root1 = self->tree1.root;
    self->count = 0;
    if (root1) {
        ScriptTreeLinks* a = root1->left;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* cachedLeft3 = b->left;
                ScriptTreeLinks* c = cachedLeft3;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cachedNode4 = c;
                    ScriptTreeLinks* cachedNode5 = cachedNode4;
                    ScriptTreeLinks* cachedNode6 = cachedNode5;
                    appFreeUni__FPv(cachedNode6);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cValue6 = c;
                    appFreeUni__FPv(cValue6);
                }
                appFreeUni__FPv(b);
            }
            ScriptTreeLinks* cachedRight7 = a->right;
            b = cachedRight7;
            if (b) {
                ScriptTreeLinks* cachedLeft8 = b->left;
                ScriptTreeLinks* cachedNode9 = cachedLeft8;
                ScriptTreeLinks* cachedLeft10 = b->left;
                ScriptTreeLinks* cachedNode11 = cachedLeft10;
                ScriptTreeLinks* c = cachedNode11;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cachedNode12 = c;
                    ScriptTreeLinks* cachedNode13 = cachedNode12;
                    ScriptTreeLinks* cachedNode14 = cachedNode13;
                    appFreeUni__FPv(cachedNode14);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        a = root1->right;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cValue7 = c;
                    appFreeUni__FPv(cValue7);
                }
                ScriptTreeLinks* cachedRight15 = b->right;
                c = cachedRight15;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cValue8 = c;
                    appFreeUni__FPv(cValue8);
                }
                appFreeUni__FPv(b);
            }
            b = a->right;
            if (b) {
                ScriptTreeLinks* cachedLeft16 = b->left;
                ScriptTreeLinks* cachedLeft17 = b->left;
                ScriptTreeLinks* cachedLeft18 = b->left;
                ScriptTreeLinks* cachedNode19 = cachedLeft18;
                ScriptTreeLinks* c = cachedNode19;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    appFreeUni__FPv(c);
                }
                ScriptTreeLinks* cachedRight20 = b->right;
                c = cachedRight20;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(&self->tree1, c->right);
                    ScriptTreeLinks* cValue9 = c;
                    appFreeUni__FPv(cValue9);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        appFreeUni__FPv(root1);
        self->tree1.size = 0;
        self->tree1.root = 0;
        self->tree1.begin = &self->tree1.root;
    }
    self->tail[0] = 0;
    self->tail[1] = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Poll queued events and keyed moves without dispatching new commands, then update effects and faces.
void EventScript_SweepPendingCallbacks(void* object) {
    // Phase 1: poll the five owning queues without dispatching any new command.
    unsigned char* o = (unsigned char*)object;
    E1B4Node* node;
    C58CSweep((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, cameraEvents)));
    C58CSweepB((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, modelEvents)));
    C58CSweepB((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, effectEvents)));
    C58CSweepB((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, messageEvents)));
    C58CSweepB((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, soundEvents)));
#define tree ((E1B4Tree*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents)))
    {
        unsigned int size = tree->size;
        int completedCount = 0;
        node = tree->begin;
        while ((E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents.root)) != node) {
            int slotComplete;
            C58CItem* it = (C58CItem*)node->item;
            if (it->obj == 0)
                slotComplete = 1;
            else if (!it->obj->check(it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = 1 + it->elapsedTicks;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    delete it->obj;
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
            C58CNext(node);
        }
        if (completedCount == size) {
            node = tree->begin;
            while ((E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents.root)) != node) {
                C58CItem* x = (C58CItem*)node->item;
                if (x) {
                    delete x;
                    node->item = 0;
                }
                C58CNext(node);
            }
            E1B4Node* a = tree->root;
            if (a) {
                E1B4Node* b = a->left;
                if (b) {
                    E1B4Node* c = b->left;
                    if (c) {
                        E1B4Node* d = c->left;
                        if (d)
                            E1B4Free(tree, d);
                        E1B4Node* cachedRight1 = c->right;
                        d = cachedRight1;
                        if (d)
                            E1B4Free(tree, d);
                        appFreeUni__FPv(c);
                    }
                    E1B4Node* cachedRight2 = b->right;
                    E1B4Node* cachedRight3 = b->right;
                    E1B4Node* cachedRight4 = b->right;
                    E1B4Node* cachedRight5 = b->right;
                    E1B4Node* cachedRight6 = b->right;
                    c = cachedRight3;
                    if (c) {
                        E1B4Node* cachedRight7 = c->right;
                        E1B4Node* d = c->left;
                        if (d)
                            E1B4Free(tree, d);
                        E1B4Node* cachedRight8 = c->right;
                        d = cachedRight8;
                        if (d)
                            E1B4Free(tree, d);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(b);
                }
                E1B4Node* cachedRight9 = a->right;
                E1B4Node* cachedRight10 = a->right;
                E1B4Node* cachedRight11 = a->right;
                E1B4Node* cachedRight12 = a->right;
                E1B4Node* cachedNode13 = cachedRight11;
                E1B4Node* cachedNode14 = cachedNode13;
                b = cachedNode14;
                if (b) {
                    E1B4Node* cachedLeft15 = b->left;
                    E1B4Node* c = cachedLeft15;
                    if (c) {
                        E1B4Node* cachedLeft16 = c->left;
                        E1B4Node* d = cachedLeft16;
                        if (d)
                            E1B4Free(tree, d);
                        E1B4Node* cachedRight17 = c->right;
                        E1B4Node* cachedRight18 = c->right;
                        d = cachedRight18;
                        if (d)
                            E1B4Free(tree, d);
                        appFreeUni__FPv(c);
                    }
                    c = b->right;
                    if (c) {
                        E1B4Node* cachedLeft19 = c->left;
                        E1B4Node* d = cachedLeft19;
                        if (d)
                            E1B4Free(tree, d);
                        d = c->right;
                        if (d)
                            E1B4Free(tree, d);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(b);
                }
                appFreeUni__FPv(a);
                tree->size = 0;
                tree->root = 0;
                tree->begin = (E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents.root));
            }
        }
    }
    EventFxMap_UpdateAttachedPositions(o + SCRIPT_MEMBER_OFFSET(ScriptEffectMapView, effectMap));
    {
        node = (*(E1B4Node**)&AsEventScriptStateView(o)->resourceTreeFirst);
        E1B4Node* end = (E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptResourceMapView, resources.root));
        while (end != node) {
            ((CEventFace*)(node->item))->UpdateTracks();
            C58CNext(node);
        }
    }
}
#undef tree
#pragma pop

// Report that this tour-versus window interface is available.
int CTourVSWindow::vfunc_1() {
    return 1;
}

#pragma push
#pragma dont_inline on
// Return inline or heap character data from a narrow MSL small string.
void* ScriptString_GetCharacterData(void* string) {
    unsigned int value = *(unsigned int*)string;
    if ((value >> 31) == 0)
        return (char*)string + 1;
    return ((NString*)string)->data;
}
#pragma pop

#pragma push
#pragma auto_inline off
#pragma dont_inline off
// Lazily initialize and return the actor slot table.
void* CourseSlotTable_GetInstance() {
    if (((unsigned int&)GetConteModelSlotsSlot()) == 0) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty(s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((unsigned int&)GetConteModelSlotsSlot()) = (unsigned int)s_conteModelSlots;
    }
    return (void*)((unsigned int&)GetConteModelSlotsSlot());
}
#pragma pop

// Read the node stored in an iterator wrapper.
void* ScriptMapIterator_GetNode(void** iterator) {
    return *iterator;
}

#pragma push
#pragma dont_inline on
// Find an exact byte key, returning the embedded end sentinel when it is absent.
void ScriptResourceMap_Find(void** result, void* map, unsigned char* soughtKey) {
    struct D070Node {
        void* left;
        void* right;
        char reserved_8[4];
        unsigned char key;
    };
    struct D070Tree {
        char reserved_0[4];
        D070Node* root;
    };
    D070Tree* t = (D070Tree*)map;
    D070Node* node = t->root;
    D070Node* cursor = (D070Node*)&t->root;
    while (node != 0) {
        unsigned char key = node->key;
        if (key >= *soughtKey) {
            cursor = node;
            node = (D070Node*)node->left;
        } else {
            node = (D070Node*)node->right;
        }
    }
    if (cursor != (D070Node*)&t->root) {
        if (*soughtKey < cursor->key)
            cursor = (D070Node*)((char*)map + 4);
    }
    *result = cursor;
}
#pragma pop

// Return the mapped-value address, inserting the default value for a missing key.
void* ScriptByteMap_GetOrInsertValue(void* map, unsigned char* key) {
    unsigned char left, updateBegin;
    // Initially the predecessor node; reused below for the selected key/value pair.
    void* candidate;
    void* node = EventFxMap_FindInsertionPosition(map, key, &candidate, &left, &updateBegin);
    if (candidate == 0 || AsEventScriptByteTreeNode(candidate)->key < *key) {
        struct {
            unsigned char key;
            unsigned int value;
        } pair;
        pair.key = *key;
        pair.value = s_scriptByteMapDefaultValue;
        node = EventFxMap_InsertNode(map, (unsigned int)node, left, updateBegin, &pair);
        candidate = (char*)&AsEventScriptByteTreeNode(node)->key;
    } else {
        candidate = (char*)&AsEventScriptByteTreeNode(candidate)->key;
    }
    return (char*)candidate + 4;
}

// Default iterator construction leaves its storage untouched.
void ScriptMapIterator_DefaultConstruct() {}

#pragma push
#pragma dont_inline on
// Find an exact byte key through a lower-bound walk, or return the end sentinel.
void ScriptByteMap_Find(void** result, void* map, unsigned char* key) {
    struct D160Node {
        void* left;
        void* right;
        char reserved_8[4];
        unsigned char key;
    };
    struct D160Tree {
        char reserved_0[4];
        D160Node* root;
    };
    D160Tree* t = (D160Tree*)map;
    D160Node* node = t->root;
    D160Node* cursor = (D160Node*)&t->root;
    while (node != 0) {
        if (node->key >= *key) {
            cursor = node;
            node = (D160Node*)node->left;
        } else {
            node = (D160Node*)node->right;
        }
    }
    if (cursor != (D160Node*)&t->root) {
        if (*key < cursor->key)
            cursor = (D160Node*)&t->root;
    }
    *result = cursor;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Copy the iterator node word.
void ScriptMapIterator_Copy(unsigned int* destination, unsigned int* source) {
    *destination = *source;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Construct the end iterator from the map embedded root/sentinel address.
void ScriptMapIterator_SetEnd(void** iterator, char* map) {
    *iterator = map + 4;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Compare the node addresses held by two iterator wrappers.
unsigned int ScriptMapIterator_NotEqual(unsigned int* left, unsigned int* right) {
    return *left != *right;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Return the byte-key/value pair inside the current map node.
char* MapIter_GetValuePtr(void** iterator) {
    return (char*)&AsEventScriptByteTreeNode(*iterator)->key;
}
#pragma pop

// Locate a byte-key insertion parent, predecessor, side, and begin-update flag.
void* EventFxMap_FindInsertionPosition(void* map, unsigned char* soughtKey, void** predecessor,
                                       unsigned char* insertLeft, unsigned char* updateBegin) {
    *predecessor = 0;
    void* node = ((ScriptByteMapView*)map)->root;
    void* cursor = (char*)&((ScriptByteMapView*)map)->root;
    *insertLeft = 1;
    *updateBegin = 1;
    while (node != 0) {
        unsigned char key = *soughtKey;
        cursor = node;
        if (key < AsEventScriptByteTreeNode(node)->key) {
            node = *(void**)node;
            *insertLeft = 1;
        } else {
            *predecessor = node;
            node = AsEventScriptByteTreeNode(node)->right;
            *insertLeft = 0;
            *updateBegin = 0;
        }
    }
    return cursor;
}

#pragma push
#pragma use_lmw_stmw on
// Allocate and link a byte-key node, update map size/begin, and rebalance the tree.
void* EventFxMap_InsertNode(register void* map, register unsigned int parentAddress, register unsigned int insertLeft,
                            register unsigned int updateBegin, register void* keyValue) {
    void** tree = (void**)map;
    if ((unsigned int)tree[0] == kScriptByteTreeCountLimit) {
        register std::length_error_8009D25C ex;
    }
    void* volatile allocated;
    void* volatile storage;
    Alloc_8009D25C alloc;
    alloc.c = 0;
    register void* node = appMallocDummy__FUl(0x14);
    storage = node;
    allocated = node;
    NodeGuard_8009D25C guard(tree + 1, node);
    Value_8009D25C* value = (Value_8009D25C*)&AsEventScriptByteTreeNode(node)->key;
    new (value) Value_8009D25C(*(Value_8009D25C*)keyValue);
    AsEventScriptByteTreeNode(node)->right = 0;
    guard.node = 0;
    *(void**)node = 0;
    unsigned int* parent = (unsigned int*)&AsEventScriptByteTreeNode(node)->parent;
    parent_8009D25C(parent, parentAddress);
    if (insertLeft)
        *(void**)parentAddress = node;
    else
        *(void**)(parentAddress + 4) = node;
    ++*(unsigned int*)tree;
    RBTree_RebalanceAfterInsert(node, (unsigned int)tree[1]);
    if (updateBegin)
        tree[2] = node;
    return node;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Append a deep copy of a narrow string, growing the vector with exception-safe ownership.
void StringVector_AppendCopy(void* vector, void* string) {
    Vec_8009D428* self = (Vec_8009D428*)vector;
    Str_8009D428* elem = (Str_8009D428*)string;
    volatile unsigned char z = 0;
    if (self->size < self->cap) {
        Str_8009D428* slot = self->first + self->size;
        new (slot) Str_8009D428(*elem);
        self->size = self->size + 1;
        return;
    }
    // Phase 1: reserve a guarded allocation; the old vector remains the source.
    if (kMaxStringVectorSize - self->cap < 1)
        LE_THROW_8009D428
    VecBuf_8009D428 buf(&self->cap);
    unsigned int minGrow = self->size + 1 - self->cap;
    unsigned int cap = self->cap;
    if (minGrow > kMaxStringVectorSize - cap)
        LE_THROW_8009D428
    unsigned int newCap;
    if (cap < kStringVectorSmallGrowthLimit) {
        unsigned int g = ((cap + 1) * 3) / 5;
        newCap = cap + max_8009D428(g, minGrow);
    } else if (cap < kStringVectorLargeGrowthLimit) {
        unsigned int g = (cap + 1) >> 1;
        newCap = cap + max_8009D428(g, minGrow);
    } else
        newCap = kMaxStringVectorSize;
    if (newCap > kMaxStringVectorSize)
        LE_THROW_8009D428
    Str_8009D428* data = (Str_8009D428*)appMallocDummy__FUl(newCap * 12);
    unsigned int sz = self->size;
    buf.first = data;
    buf.cap = newCap;
    buf.used = sz;
    {
        Str_8009D428* p = data + sz + buf.count;
        for (unsigned int k = 1; k; --k) {
            new (p) Str_8009D428(*elem);
            ++p;
            buf.count = buf.count + 1;
        }
    }
    // Phase 2: appended copy is live; transfer existing representations to the
    // preceding slots and zero their old storage to prevent double destruction.
    Str_8009D428* oldItems = self->first;
    unsigned int n = (oldItems + self->size) - oldItems;
    buf.used = buf.used - n;
    memcpy(buf.first + buf.used, oldItems, n * 12);
    memset(oldItems, 0, n * 12);
    buf.count = buf.count + n;
    {
        unsigned int oldSize = self->size;
        Str_8009D428* of = self->first;
        unsigned int oc = self->cap;
        Str_8009D428* nf = buf.first;
        unsigned int total = buf.count;
        unsigned int nc = buf.cap;
        Str_8009D428* begin = of + buf.used;
        buf.first = of;
        self->cap = nc;
        buf.cap = oc;
        self->first = nf;
        self->size = total;
        buf.count = oldSize;
        Str_8009D428* end = begin + oldSize;
        while (end > begin) {
            --end;
            if (end)
                end->~Str_8009D428();
        }
        buf.count = 0;
    }
}
#pragma pop

// Clear the owned-buffer count, free its data, and optionally delete the object.
int ScriptOwnedBuffer_dtor(void* buffer, int deleteStorage) {
    if (buffer != 0) {
        void* zero = 0;
        (*(void**)&AsScriptOwnedBuffer80092804(buffer)->count) = zero;
        if (buffer != 0) {
            if (AsScriptOwnedBuffer80092804(buffer)->data != 0) {
                (*(void**)&AsScriptOwnedBuffer80092804(buffer)->count) = zero;
                appFreeUni__FPv(AsScriptOwnedBuffer80092804(buffer)->data);
            }
        }
        if (deleteStorage > 0)
            __dl__FPv(buffer);
    }
    return (int)buffer;
}

#pragma push
#pragma use_lmw_stmw on
// Copy the selected character suffix used to build a tour voice resource name.
void Character_GetAssetNameSuffix(Fn8009BBFCString* out, signed char index) {
    Fn8009DB08String strings[8] = {
        Fn8009DB08String(s_tourVoiceKenSuffix, 1),     Fn8009DB08String(s_tourVoiceErikaSuffix, 1),
        Fn8009DB08String(s_tourVoiceDaisukeSuffix, 1), Fn8009DB08String(s_tourVoiceCecilliaSuffix),
        Fn8009DB08String(s_tourVoiceMaxSuffix),        Fn8009DB08String(s_tourVoiceKoohSuffix),
        Fn8009DB08String(s_tourVoiceArinSuffix),       Fn8009DB08String(s_tourVoiceKazSuffix)};
    Fn8009DB08String* selected = strings + (int)index;
    selected->copyTo(out);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Copy one of the decide/win/lose/change/item/versus voice suffixes.
void Event_GetAssetNameSuffix(Fn8009BBFCString* out, int index) {
    Fn8009DEC4String strings[6] = {s_tourVoiceDecideSuffix, s_tourVoiceWinSuffix,  s_tourVoiceLoseSuffix,
                                   s_tourVoiceChangeSuffix, s_tourVoiceItemSuffix, s_tourVoiceVersusSuffix};
    Fn8009DEC4String* selected = strings + index;
    if ((selected->w0 >> 31) == 0) {
        unsigned int len = selected->len;
        AsMWStringStorage(out)->flags = selected->w0;
        AsMWStringStorage(out)->length = len;
        *(unsigned int*)&out->data[8] = selected->heapAddressOrInlineTail;
    } else {
        AsMWStringStorage(out)->flags = 0;
        AsMWStringStorage(out)->length = 0;
        *(unsigned int*)&out->data[8] = 0;
        String_ReserveCapacity(out, selected->len);
        Fn8009DB08Tag tag;
        Fn8009DB08Tag ch;
        ch.value = tag.value;
        ((ScriptDiagnosticRangeCall128)String_ReplaceRange)(
            out, 0, 0, (const char*)selected->heapAddressOrInlineTail,
            (const char*)selected->heapAddressOrInlineTail + selected->len, &ch);
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Free retired keyed-move links in postorder after their wrappers are destroyed.
// head is the embedded root-slot sentinel; snapshots borrow nodes until their free.
static inline void ReleaseCompletedKeyedMoveLinks(unsigned char* o, E1B4Node* head) {
#define tree ((E1B4Tree*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents)))
    // Link teardown: postorder (left descendants, right descendants, root).
    // This is compiler-expanded container machinery, not opcode policy.
    // Seven link snapshots retain the required register lifetimes; no snapshot owns a node.
    // Empty sentinel is head. All other redundant snapshots were removed.
    E1B4Node* retiredRoot = tree->root;
    if (retiredRoot) {
        E1B4Node* firstBranchSnapshot = retiredRoot->left;
        E1B4Node* retiredBranch = firstBranchSnapshot;
        if (retiredBranch) {
            E1B4Node* leftChildSnapshot = retiredBranch->left;
            E1B4Node* retiredChild = leftChildSnapshot;
            if (retiredChild) {
                E1B4Node* leftLeafSnapshot = retiredChild->left;
                E1B4Node* retiredLeaf = leftLeafSnapshot;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                retiredLeaf = retiredChild->right;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                appFreeUni__FPv(retiredChild);
            }
            retiredChild = retiredBranch->right;
            if (retiredChild) {
                E1B4Node* leftLeafSnapshot = retiredChild->left;
                E1B4Node* retiredLeaf = leftLeafSnapshot;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                retiredLeaf = retiredChild->right;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                appFreeUni__FPv(retiredChild);
            }
            appFreeUni__FPv(retiredBranch);
        }
        retiredBranch = retiredRoot->right;
        if (retiredBranch) {
            E1B4Node* leftChildSnapshot = retiredBranch->left;
            E1B4Node* retiredChild = leftChildSnapshot;
            if (retiredChild) {
                E1B4Node* retiredLeaf = retiredChild->left;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                E1B4Node* rightLeafSnapshot = retiredChild->right;
                retiredLeaf = rightLeafSnapshot;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                appFreeUni__FPv(retiredChild);
            }
            retiredChild = retiredBranch->right;
            if (retiredChild) {
                E1B4Node* leftLeafSnapshot = retiredChild->left;
                E1B4Node* retiredLeaf = leftLeafSnapshot;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                retiredLeaf = retiredChild->right;
                if (retiredLeaf)
                    E1B4Free(tree, retiredLeaf);
                appFreeUni__FPv(retiredChild);
            }
            appFreeUni__FPv(retiredBranch);
        }
        appFreeUni__FPv(retiredRoot);
        // Restore the empty root-slot sentinel only after the last node dies.
        tree->begin = head;
        tree->root = 0;
        tree->size = 0;
    }
#undef tree
}

// Poll all keyed moves; retire wrappers/links together only when every slot completes.
// Completed event payloads die immediately; ticks saturate for events still waiting.
static inline void SweepKeyedModelCallbacks(unsigned char* o) {
#define tree ((E1B4Tree*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents)))
    // Poll keyed moves. Finished callbacks are destroyed immediately,
    // but wrappers/nodes stay until ALL entries are complete, then the map clears.
    E1B4Node* head = (E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, keyedModelEvents.root));
    {
        int completedCount = 0;
        int n = tree->size;
        E1B4Node* node = tree->begin;
        while (head != node) {
            int slotComplete;
            E1B4Item* it = node->item;
            if (!it->obj)
                slotComplete = 1;
            else if (!it->obj->vt->check(it->obj, it->elapsedTicks)) {
                if (it->elapsedTicks < kSaturatedCommandTick)
                    it->elapsedTicks = 1 + it->elapsedTicks;
                slotComplete = 0;
            } else {
                if (it->obj) {
                    if (it->obj)
                        it->obj->vt->dtor(it->obj, 1);
                    it->obj = 0;
                }
                slotComplete = 1;
            }
            if (slotComplete)
                ++completedCount;
            node = E1B4Next(node);
        }
        // Retirement gate: do not erase any map node until every callback is done.
        // First destroy wrappers in key order; only then free the tree links.
        if (n == completedCount) {
            node = tree->begin;
            while (head != node) {
                E1B4Sub* x = (E1B4Sub*)node->item;
                if (x) {
                    if (x)
                        x->vt->dtor(x, 1);
                    node->item = 0;
                }
                node = E1B4Next(node);
            }
            ReleaseCompletedKeyedMoveLinks(o, head);
        }
    }
#undef tree
}

// Run opcodes until blocked/exhausted, then poll callbacks and update attached effects.
// Execution states: handler 0 -> keep cursor/ticks for next call; handler 1 ->
// handler advances cursor and dispatcher resets ticks; null cursor -> no dispatch.
// Polling runs in ALL three states. Return reports dispatch only, not quiescence.
// Dispatch until blocked, poll all callback owners, and update retained effect/face attachments.
int EventScript_RunAndSweep(void* object) {
    // Return 0 when an opcode waits, 1 when the cursor is exhausted. Queue/map
    // polling still runs after a wait; 1 alone does not mean all callbacks ended.
    // Phase 1: dispatch commands; a successful handler resets per-command ticks.
    unsigned int result = 1;
    unsigned char* o = (unsigned char*)object;
    while ((*(void**)&AsEventScriptStateView(o)->cursor)) {
        if (!((int (*)(void*))scriptOpcodeHandlers[*(unsigned char*)((*(void**)&AsEventScriptStateView(o)->cursor))])(
                o)) {
            result = 0;
            break;
        }
        AsEventScriptStateView(o)->elapsedCommandTicks = 0;
    }
    // Phase 2: sweep each owning callback queue in camera/model/effect/message/
    // sound order. Each specialization retains its original erase/poll schedule.
    C58CSweep((C58CVec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, cameraEvents)));
    E1B4Sweep((E1B4Vec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, modelEvents)));
    E1B4Sweep((E1B4Vec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, effectEvents)));
    E1B4Sweep((E1B4Vec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, messageEvents)));
    E1B4Sweep((E1B4Vec*)(o + SCRIPT_MEMBER_OFFSET(ScriptQueueView, soundEvents)));
    // Phase 3: keyed moves retain nodes until every callback has completed.
    SweepKeyedModelCallbacks(o);
    // Phase 4: update effect attachment positions and retained face tracks even
    // if command dispatch stopped on a wait. Resource-map faces are not erased here.
    EventFxMap_UpdateAttachedPositions(o + SCRIPT_MEMBER_OFFSET(ScriptEffectMapView, effectMap));
    {
        E1B4Node* end = (E1B4Node*)(o + SCRIPT_MEMBER_OFFSET(ScriptResourceMapView, resources.root));
        E1B4Node* node = (*(E1B4Node**)&AsEventScriptStateView(o)->resourceTreeFirst);
        while (node != end) {
            ((CEventFace*)(node->item))->UpdateTracks();
            node = E1B4Next(node);
        }
    }
    return result;
}
#undef tree
#pragma pop

// Unrolled tree frees and capture lifetimes preserve the original register allocation.
void ScriptSharedFiles_ResetRequiredFilesAndHandlers() {
    // Phase 1: release handler-tree nodes and restore the empty sentinel.
    char* mapSentinelAddress1 = 4 + s_scriptByteKeyMap;
    ScriptTreeLinks* root = (ScriptTreeLinks*)*(void**)(mapSentinelAddress1);
    if (root) {
        ScriptTreeLinks* a = root->left;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* cachedLeft2 = b->left;
                ScriptTreeLinks* c = cachedLeft2;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            ScriptTreeLinks* cachedRight3 = a->right;
            b = cachedRight3;
            if (b) {
                ScriptTreeLinks* cachedLeft4 = b->left;
                ScriptTreeLinks* c = cachedLeft4;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        ScriptTreeLinks* cachedRight5 = root->right;
        a = cachedRight5;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            ScriptTreeLinks* cachedRight6 = a->right;
            b = cachedRight6;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                c = b->right;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree(s_scriptByteKeyMap, c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        appFreeUni__FPv(root);
        *(void**)s_scriptByteKeyMap = 0;
        *(void**)(4 + s_scriptByteKeyMap) = 0;
        *(void**)(8 + s_scriptByteKeyMap) = (4 + s_scriptByteKeyMap);
    }
    // Phase 2: register the byte-17 binding against the now-empty handler map.
    unsigned char left;
    // Initially the predecessor node; reused below for the selected key/value pair.
    void* candidate;
    void* localNode;
    unsigned char updateBegin;
    void* found = EventFxMap_FindInsertionPosition(s_scriptByteKeyMap, (unsigned char*)&scriptByteHandlerBinding_17,
                                                   &candidate, &left, &updateBegin);
    if (!candidate || AsEventScriptByteTreeNode(candidate)->key < scriptByteHandlerBinding_17) {
        unsigned int localValue;
        unsigned char key = scriptByteHandlerBinding_17;
        localValue = s_scriptByteMapDefaultValue;
        unsigned int localValueValue = localValue;
        struct {
            unsigned char k;
            unsigned int v;
        } localPair;
        localPair.k = scriptByteHandlerBinding_17;
        localPair.v = localValue;
        localNode = EventFxMap_InsertNode(s_scriptByteKeyMap, (unsigned int)found, left, updateBegin, &localPair);
        candidate = (char*)&AsEventScriptByteTreeNode(localNode)->key;
    } else
        candidate = (char*)&AsEventScriptByteTreeNode(candidate)->key;
    unsigned int count = ((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->size;
    AsEventScriptByteTreePair(candidate)->value = AsEventScriptByteTreePair(&scriptByteHandlerBinding_17)->value;
    // Phase 3: destroy owned required-file strings, keeping their vector buffer.
    unsigned char* item =
        (unsigned char*)((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->data + 12 * count;
    ((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->size = count - count;
    for (; count != 0; --count) {
        item -= 12;
        if ((*(unsigned int*)item) >> 31)
            appFreeUni__FPv(*(void**)(item + 8));
    }
}

#pragma push
#pragma use_lmw_stmw on
// Dispatch shared-file names and ordinary commands on a required initialized
// interpreter. applyLoads is a BOOLEAN TOKEN (null=0, nonnull=1), not a pointer
// to read. Required-file names are always copied into the global name vector;
// the token is forwarded only to SharedFileList_CreateAndAppend's load policy.
// Precondition: reset/register shared handlers first; every non-file opcode
// must index a valid scriptOpcodeHandlers entry (byte domain 0..255, unchecked).
// Shared handler ABI is (script, temporaryString)->int: zero retries immediately
// and must eventually advance or change state; nonzero publishes its name.
// Ordinary handler ABI is (script)->int: zero yields 0 with its cursor/pending
// state intact; nonzero consumes/advances. End opcode 0x0A clears cursor and
// returns 1; an already-null cursor also returns 1. Both paths sweep callbacks.
// Completion means parsing finished, NOT files loaded: poll
// StoryConte_AreRequiredFilesAvailable separately. Publication is list append
// THEN required-name deep copy; throws may leave a partial append, no rollback.
// Local name storage is destroyed on exit, so callees must copy/retain their
// own data. Global handler/name/list storage and callback sweeping prohibit
// concurrent use or reentry that resets/removes those globals.
int ScriptState_ProcessSharedFileCommands(void* script, void* applyLoads) {
    Fn8009F028Local fileName;
    unsigned int handlerIterator;
    void* foundIterator;
    void* endIterator;
    unsigned char opcode;
    int apply;
    int completed;
    ((void (*)(void*))ScriptMapIterator_DefaultConstruct)(&handlerIterator);
    Storage_ClearThreeWords(&fileName);
    completed = 1;
    apply = (applyLoads != 0);
    while ((AsEventScriptStateView(script)->cursor) != 0) {
        unsigned char* cursor = AsEventScriptStateView(script)->cursor;
        opcode = AsEventScriptCommandView(cursor)->opcode;
        if (opcode == kSharedFileEndOpcode) {
            (*(unsigned int*)&AsEventScriptStateView(script)->cursor) = 0;
            break;
        }
        ScriptByteMap_Find(&foundIterator, s_scriptByteKeyMap, &opcode);
        ScriptMapIterator_Copy((unsigned int*)&handlerIterator, (unsigned int*)&foundIterator);
        ScriptMapIterator_SetEnd(&endIterator, s_scriptByteKeyMap);
        if (ScriptMapIterator_NotEqual((unsigned int*)&handlerIterator, (unsigned int*)&endIterator) != 0) {
            int res = ((int (*)(void*, void*)) *
                       (void**)(MapIter_GetValuePtr((void**)&handlerIterator) + 4))(script, &fileName);
            if (res == 0)
                continue;
            SharedFileList_CreateAndAppend(ScriptState_GetSharedFileList(), ScriptString_GetCharacterData(&fileName),
                                           apply);
            StringVector_AppendCopy((void*)s_storyConteRequiredFilesStorage, &fileName);
        } else {
            void* handler = scriptOpcodeHandlers[opcode];
            int res = ((int (*)(void*))handler)(script);
            if (res == 0) {
                completed = 0;
                break;
            }
        }
        AsEventScriptStateView(script)->elapsedCommandTicks = 0;
    }
    EventScript_SweepPendingCallbacks(script);
    return completed;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Temporarily set the current nesting flag and run shared-file commands until complete.
// index must be 0..15 for the u16 flags field; do not enter with that bit already
// set (normal exit clears it rather than restoring the old value). Busy-retries
// parser waits without advancing external time. Throws leave the nesting bit
// set: there is no scope guard. Same global/reentrancy contract as dispatch.
int ScriptState_RunSharedFileCommandsUntilComplete(Fn8009F184Arg* script, void* applyLoads) {
    script->flags |= 1 << script->index;
    // Dispatch commands, collect requested files, stop at EV_EXIT or a wait, then sweep callbacks.
    while (ScriptState_ProcessSharedFileCommands(script, applyLoads) == 0) {
    }
    script->flags &= ~(1 << script->index);
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return whether every required name resolves to a NONNULL LOADED BUFFER in
// the shared file list (FileList_GetLoadedBufferByName), not a file-size test.
// Empty requirement vector returns1 without creating the list. Otherwise lazily
// initialize the shared list, stop at the first null lookup, and return0. Names
// are borrowed for the lookup; no buffer is retained or released by this query.
int StoryConte_AreRequiredFilesAvailable() {
    if (((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->size == 0)
        return 1;
    unsigned char* item = (unsigned char*)((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->data;
    for (; item != (unsigned char*)((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->data +
                       ((ScriptStringVectorView*)s_storyConteRequiredFilesStorage)->size * 12;
         item += 12) {
        if (((unsigned int&)Sing_805DC680()) == 0) {
            if (s_sharedFileListInitialized == 0) {
                SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                __register_global_object(s_sharedFileListStorage, (void (*)())SharedFilePointerVector_Destroy,
                                         s_sharedFilePointerVectorDtorRegistration);
                s_sharedFileListInitialized = 1;
            }
            ((unsigned int&)Sing_805DC680()) = (unsigned int)s_sharedFileListStorage;
        }
        const char* name = ((*(unsigned int*)item >> 31) == 0) ? (const char*)(item + 1) : *(const char**)(item + 8);
        if (!FileList_GetLoadedBufferByName((void*)((unsigned int&)Sing_805DC680()), name))
            return 0;
    }
    return 1;
}
#pragma pop

// Clear file handlers, remove required files from the shared list, and free the name vector.
void ScriptSharedFiles_RemoveRequiredFilesAndRelease() {
    // Phase 1: free handler-map nodes and restore its empty sentinel.
    char* base = s_storyWindowStaticStorage;
    ScriptTreeLinks* root = ((ScriptTreeLinks**)((base + 0x48)))[1];
    if (root) {
        ScriptTreeLinks* cachedLeft1 = root->left;
        ScriptTreeLinks* a = cachedLeft1;
        if (a) {
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                ScriptTreeLinks* cachedRight2 = b->right;
                c = cachedRight2;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            b = a->right;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                ScriptTreeLinks* cachedRight3 = b->right;
                c = cachedRight3;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        a = root->right;
        if (a) {
            ScriptTreeLinks* cachedLeft4 = a->left;
            ScriptTreeLinks* cachedNode5 = cachedLeft4;
            ScriptTreeLinks* b = a->left;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                ScriptTreeLinks* cachedRight6 = b->right;
                c = cachedRight6;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            b = a->right;
            if (b) {
                ScriptTreeLinks* c = b->left;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                ScriptTreeLinks* cachedRight7 = b->right;
                c = cachedRight7;
                if (c) {
                    if (c->left)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->left);
                    if (c->right)
                        ScriptByteMap_FreeSubtree((ScriptTreeLinks**)((base + 0x48)), c->right);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
        appFreeUni__FPv(root);
        *(void**)((base + 0x48)) = 0;
        ((ScriptTreeLinks**)((base + 0x48)))[1] = 0;
        ((ScriptTreeLinks**)((base + 0x48)))[2] = (ScriptTreeLinks*)(&((SharedFileCleanupView*)base)->handlers.root);
    }
    // Phase 2: remove each required name from the shared-file list, clear the
    // string contents, then free its owned heap representation if present.
    Fn8009F2E4Item** vec = (Fn8009F2E4Item**)((base + 0x60));
    int count = *(unsigned int*)(vec + 1);
    if (count) {
        Fn8009F2E4Item* item = *vec;
        while (item != *vec + count) {
            if (!((unsigned int&)Sing_805DC680())) {
                if ((signed char)s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void (*)())SharedFilePointerVector_Destroy,
                                             s_sharedFilePointerVectorDtorRegistration);
                    s_sharedFileListInitialized = 1;
                }
                ((unsigned int&)Sing_805DC680()) = (unsigned int)s_sharedFileListStorage;
            }
            const char* key = (item->flag >> 31) ? (const char*)item->ptr : (const char*)item + 1;
            SharedFileHandleList_RemoveByDataName((void*)((unsigned int&)Sing_805DC680()), key);
            if (!(item->flag >> 31)) {
                *(unsigned char*)((char*)item + 1) = 0;
                *(unsigned char*)item &= 0x80;
            } else {
                *(unsigned char*)item->ptr = 0;
                item->value = 0;
            }
            item = item + 1;
        }
        *(unsigned int*)(vec + 1) = 0;
        while (0 != count) {
            count = count - 1;
            Fn8009F2E4Item* p = *vec + count;
            if (p->flag >> 31)
                appFreeUni__FPv(p->ptr);
        }
    }
    // Phase 3: detach the vector buffer and release any remaining elements and
    // storage. Its size was cleared by Phase 2 when there were live names.
    Fn8009F2E4Item* data = *vec;
    int size = *(unsigned int*)(1 + vec);
    *(unsigned int*)(2 + vec) = 0;
    *vec = 0;
    *(unsigned int*)(vec + 1) = 0;
    if (data) {
        while (size != 0) {
            --size;
            Fn8009F2E4Item* p = (size + data);
            if (p->flag >> 31)
                appFreeUni__FPv(p->ptr);
        }
        appFreeUni__FPv(data);
    }
}

#pragma push
#pragma dont_inline on
// Lazily initialize and return the shared file-handle list.
void* ScriptState_GetSharedFileList() {
    if (*(volatile unsigned int*)&((unsigned int&)Sing_805DC680()) == 0) {
        if (s_sharedFileListInitialized == 0) {
            char* base = s_sharedFileListStorage;
            SharedFilePointerVector_InitReserved1024(base);
            __register_global_object(base, (void (*)())SharedFilePointerVector_Destroy,
                                     s_sharedFilePointerVectorDtorRegistration);
            s_sharedFileListInitialized = 1;
        }
        ((unsigned int&)Sing_805DC680()) = (unsigned int)s_sharedFileListStorage;
    }
    return (void*)((unsigned int&)Sing_805DC680());
}
#pragma pop

// Destroy owned buffer contents and optionally release the buffer object itself.
int SharedFilePointerVector_Destroy(void* buffer, int deleteStorage) {
    if (buffer != 0) {
        if (buffer != 0) {
            if (buffer != 0) {
                if (buffer != 0) {
                    if (buffer != 0) {
                        if (buffer != 0) {
                            void* value = AsScriptOwnedBuffer80092804(buffer)->data;
                            if (value != 0) {
                                int old = (*(volatile int*)&AsScriptOwnedBuffer80092804(buffer)->count);
                                AsScriptOwnedBuffer80092804(buffer)->count = old - old;
                                appFreeUni__FPv(value);
                            }
                        }
                    }
                }
            }
        }
        if (deleteStorage > 0)
            __dl__FPv(buffer);
    }
    return (int)buffer;
}
} // extern "C"
