// Script effects and sounds: the byte-keyed effect map, sound and BGM script commands, and the
// pad swing sensor (CPadSwingAccele) with its motion controller (CPadSwingCtrlMotion).

#include <decomp/static_init.h>
#include "game/gtl_input.h"
#include <decomp/record_sizes.h>
#include "classes/CEventFx.h"
#include "classes/CEventFxPositionLayout.h"
#include "game/singletons.h"
#include "game/string.h"
#include "game/eventconte.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CPadSwingAccele.h"
#include "classes/CPadSwingCtrlMotion.h"
#include "classes/CEventSoundSC.h"
#include "classes/CEventSoundSC_SND_SE_PITCH.h"
#include "classes/CEventEffect.h"

// View borrowed event-effect storage without adjusting its address.
static inline CEventEffect* AsCEventEffect(void* p) {
    return (CEventEffect*)p;
}

// View common script-sound storage at the supplied base address.
static inline CEventSoundSC* AsCEventSoundSC(void* p) {
    return (CEventSoundSC*)p;
}

// View the shared swing storage underlying this unit's phase-specific layouts.
static inline CPadSwingAccele* AsCPadSwingAccele(void* p) {
    return (CPadSwingAccele*)p;
}

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
#define SWING_OFFSET(field) ((int)&((CPadSwingAccele*)0)->field)

enum {
    kSampleBatchCapacity = 32,
    kShortHistorySamples = 30,
    kLongHistorySamples = 120,
    kCaptureCapacity = 512,
    kPeakHistorySize = 8
};
enum {
    kSwingVectorBytes = 12,
    kPeakWindowSamples = 5,
    kDirectionWindowSamples = 40,
    kBiasHistorySkip = 20,
    kLongHistoryOffset = 0x310,
    kCaptureReservedEndpoint = kCaptureCapacity - 1
};
enum {
    kCaptureTimeoutGraceFrames = 2,
    kLastLongHistorySlot = kLongHistorySamples - 1
};
static const float kCaptureTriggerZ = 0.8f;      // Raw device acceleration units.
static const float kCapturePeakToTroughX = 0.5f; // Raw device acceleration difference.
// Linked constants and helpers used by the member functions below. The namespace blocks declare the same symbols.
extern "C" {
extern const float kFloat_0_0_SwingAcceleration;
extern void Vec3_SetAll(void*, float);
extern const float kFloat_neg0_001_PadSwingAcceleration;
extern const float kSwingDirectionYSumUpperBound;
extern const float kFloat_15_SwingAccelerometer;
extern const float kFloat_neg1_swingAccelerometer;
extern const float kFloat_0_8_SwingAnalysis;
extern const float kFloat_0_805E0078;
extern char __vt__19CPadSwingCtrlMotion[];
extern char swingMotionNames[];
void __dla__FPv(void*);
}
// Buffer/time model: packed XYZ float triplets have 12-byte stride. A pad batch
// arrives newest first (up to 32); resampling turns it into the timing-mode count
// (10 or 12), preserving a previous-batch endpoint. Both history rings append in
// reverse batch order, oldest to newest; their index points to the NEXT write.
// shortHistory holds 30 samples, longHistory 120. They are sample windows, not
// fixed durations without the pad/update rate. currentAcceleration at +300 stores
// the short-window mean in X,Z,Y order. Raw acceleration units are device-defined;
// clamping is +/-2.9. Capture stores chronological samples in 512-vector buffers,
// with slot 511 reserved for a closing endpoint. Capture elapsed time is converted
// to 30-Hz frame units for its limit tests; sample counts and frames differ.
enum SwingCaptureState {
    kCaptureIdle = 0,
    kCaptureArmed = 1,
    kCaptureTriggered = 2,
    kCaptureComplete = 3
};
enum SwingMotionState {
    kMotionIdle = 0,
    kMotionReturning = 1,
    kMotionForward = 2,
    kMotionSettling = 3,
    kMotionComplete = 4
};
// Two independent state domains; do not compare their numeric values to each other.
// Capture: reset -> Idle; arm -> Armed; update -> Triggered
// on the Z/X edge or elapsedSeconds*30>=2. A closing sample runs repair then
// analysis and enters Complete. A nonzero frame limit also closes at limit+2;
// zero disables that timeout. An enabled update already Triggered can cancel to
// Complete with count=0. Idle/Complete updates return without advancing storage.
// Motion: the advance step moves Idle->Returning->Forward->Settling->Complete->Idle;
// cancelling forward motion goes Forward->Returning, and reset goes to Idle. Those methods
// clear their own motion timers, not captureElapsed or captureFrameLimit.
// No capture->motion transition call is present in this unit. The owner drives
// both interfaces; changing capture timeout does not itself advance motion.

enum {
    kPitchUnityPercent = 100,
    kMinimumPitchDelta = -100,
    kMaximumPitchDelta = 9900
};

// Fade command object; its class is defined in event_script_model_commands.cpp. Only its deleting destructor is here.
struct CEventEffectSC_EFF_FADE : CEventConte {
    // Optionally delete this fade command; it owns no additional resources.
    void* Destroy(int deletingFlag);
};

namespace ns0 {

extern "C" {

void ScriptByteMap_FreeSubtree(void*, void*);
void appFreeUni__FPv(void*);
void* __dl__FPv(void*);
extern const char __vt__12CEventEffect[];

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    unsigned int parent;
    unsigned char key;
    char padD[3];
    void* value;
};

struct DtorNode {
    DtorNode* a;
    DtorNode* b;
};
struct ValueVtbl {
    char pad[8];
    void (*release)(void*, int);
};
struct ValueObject {
    char pad[0x38];
    ValueVtbl* vtbl;
};
// Release both child subtrees and the node storage.
static void destroyLeafDtorNode(void* owner, DtorNode* node) {
    if (!node)
        return;
    if (node->a)
        ScriptByteMap_FreeSubtree(owner, node->a);
    if (node->b)
        ScriptByteMap_FreeSubtree(owner, node->b);
    appFreeUni__FPv(node);
}
// Expand one level of subtree cleanup while retaining the original compiler inlining depth.
static void destroyLevelOneDtorNode(void* owner, DtorNode* node) {
    if (!node)
        return;
    destroyLeafDtorNode(owner, node->a);
    destroyLeafDtorNode(owner, node->b);
    appFreeUni__FPv(node);
}
// Expand two levels of subtree cleanup while retaining the original compiler inlining depth.
static void destroyLevelTwoDtorNode(void* owner, DtorNode* node) {
    if (!node)
        return;
    destroyLevelOneDtorNode(owner, node->a);
    destroyLevelOneDtorNode(owner, node->b);
    appFreeUni__FPv(node);
}

#pragma use_lmw_stmw on

extern const char __vt__24CEventSoundSC_SND_SE_REQ[];
extern const char __vt__25CEventSoundSC_SND_SE_PLAY[];
extern const char __vt__25CEventSoundSC_SND_SE_STOP[];
extern const float kEventDurationScaleNumerator_SoundScript;
extern const float kEventDurationScaleDenominator_SoundScript;
int EventTiming_IsDurationScalingEnabled();
unsigned int String_ReplaceRange(void*, int, unsigned int, const char*, const char*, char*);

struct ObjCtorBase {
    void* vtable;
    unsigned short value;
    char pad[2];
    unsigned int list;
    unsigned int capacity;
    unsigned int padding10;
    unsigned int padding14;
};

// Convert script frame counts for the active video timing mode; preserve counts zero and one.
static inline unsigned int scaleTargetCount(unsigned int count) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (count < 2)
            return count;
        return Runtime_ConvertDoubleToUnsigned((kEventDurationScaleNumerator_SoundScript * (float)count) /
                                               kEventDurationScaleDenominator_SoundScript);
    }
    return count;
}

typedef std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > SoundCommandString;

// Assign borrowed NUL-terminated text, retaining the string's existing storage policy.
// High bit selects heap storage; otherwise the first byte holds seven-bit length
// and characters begin at byte 1. ReplaceRange's third argument is OLD LENGTH,
// not capacity. The otherwise unused allocator byte preserves the original ABI.
static inline void AssignSoundCommandName(SoundCommandString& text, const char* soundName) {
    unsigned int existingLength;
    if ((text.flags >> 31) == 0)
        existingLength = *(unsigned char*)&text & 0x7f;
    else
        existingLength = text.length;
    volatile struct {
        char value;
        char padding[3];
    } allocatorStorage;
    char allocatorByte = allocatorStorage.value;
    unsigned int length = strlen(soundName);
    String_ReplaceRange(&text, 0, existingLength, soundName, soundName + length, &allocatorByte);
}

// Serialized sound commands: byte 1 is play's loop flag, +2 is the unsigned
// start frame, +4 is a borrowed NUL-terminated name. Constructors copy the name;
// execution clears its contents but retains any heap allocation for destruction.
struct EventSoundSeReq : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    EventSoundSeReq(const char* arg);
    // At the start frame, request the named sound once, reset pitch, and clear the command's name.
    int PlayNamedSoundOnce(unsigned int frame);
    // Release any heap-backed name and optionally delete the object. A named member, not a
    // destructor: a real destructor would also tear down the name string.
    void* Destroy(int deletingFlag);
};

// Decode the request start frame and copy its sound name.

// Convert script frame counts for the active video timing mode; preserve counts zero and one.
static inline unsigned int scaleCtorCount(unsigned int count) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (count < 2)
            return count;
        return Runtime_ConvertDoubleToUnsigned((kEventDurationScaleNumerator_SoundScript * (float)count) /
                                               kEventDurationScaleDenominator_SoundScript);
    }
    return count;
}

struct EventSoundSeStop : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    EventSoundSeStop(const char* arg);
    // At the start frame, stop the named sound alias, reset pitch, and clear the command's name.
    int StopSoundAlias(unsigned int frame);
    // Release any heap-backed name and optionally delete the object (see EventSoundSeReq::Destroy).
    void* Destroy(int deletingFlag);
};

// Decode the stop start frame and copy its sound alias.

struct EventSoundSePlay : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    unsigned int flag;
    EventSoundSePlay(const char* arg);
    // At the start frame, play the sound with its alias/loop flag, then reset pitch and clear the name.
    int TryPlaySoundEffect(unsigned int frame);
    // Release any heap-backed name and optionally delete the object (see EventSoundSeReq::Destroy).
    void* Destroy(int deletingFlag);
};

// Decode the play start frame, sound name, and loop flag.

extern const float kFloat_6_2831855;
extern float s_eventSoundEffectsFullTurn;

// Initialize this script/effect unit's full-turn constant.
void EventSoundEffects_InitFullTurn();

void* __dl__FPv(void*);

extern void* g_soundManager;
extern char s_eventSoundEffectPitchPercent;
extern const float kFloat_0_5_EventSound;
extern const float kFloat_200_EventFx;
extern const float kFloat_1_EventFx;
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void Sound_StopByAlias(void*, const char*);
void __pl__FRC7WVectorRC7WVector(float*, const float*, const float*);
void __as__7WVectorFRC7WVector(float*, const float*);

// Clear a sound name while keeping its inline/heap allocation for later destruction.
// heapStorage is the freshly read high storage bit after the sound-manager call.
static inline void ClearSoundCommandName(SoundCommandString& text, unsigned int heapStorage) {
    if (heapStorage == 0) {
        char* stringBytes = (char*)&text.flags;
        stringBytes[1] = 0;
        int storageAndLength = stringBytes[0];
        storageAndLength &= ~0x7F;
        stringBytes[0] = (char)storageAndLength;
    } else {
        char* stringBytes = text.data;
        stringBytes[0] = 0;
        text.length = 0;
    }
}

// Update attached effect positions by adding each bone/source position to its configured offset.
void EventFxMap_UpdateAttachedPositions(CEventEffect* self);

// treeRoot (+4) is the search root; its address doubles as the end sentinel.
// firstNode (+8) caches the first iterator and advances to treeNext on erase.
struct Tree {
    unsigned int count;
    TreeNode* treeRoot;
    TreeNode* firstNode;
};
struct VirtValue {
    char pad[0x38];
    virtual ~VirtValue();
};
void RBTree_RebalanceAfterErase(TreeNode*, TreeNode*);
void appFreeUni__FPv(void*);

// Advance the event-effect tree iterator, masking the red/black color bit in parent links.
static inline TreeNode* treeNext(TreeNode* candidate) {
    TreeNode* x = candidate->right;
    if (x) {
        while (x->left)
            x = x->left;
        candidate = x;
    } else {
        TreeNode* p;
        // 0x80221980 unit: walk parents until the left-child link matches.
        while ((p = (TreeNode*)((unsigned int)candidate->parent & ~1u)), candidate != p->left) {
            candidate = p;
        }
        candidate = p;
    }
    return candidate;
}

// Erase and free a tree node, advance the first iterator if needed, and decrement the count.
static inline void treeErase(Tree* self, TreeNode* it) {
    TreeNode* erased = it;
    if (it == self->firstNode) {
        it = treeNext(it);
        self->firstNode = it;
    }
    RBTree_RebalanceAfterErase(erased, self->treeRoot);
    appFreeUni__FPv(erased);
    --self->count;
}

#pragma opt_lifetimes off
// Delete the effect under this byte key and erase its map node if present.
void EventFxMap_EraseByKey(Tree* self_, unsigned char key);
#pragma opt_lifetimes reset

void SoundManager_SetNamedEffectPitch(void*, const char*, float);
void* __nw__FUl(unsigned int);
void* EventFxMap_FindInsertionPosition(void*, unsigned char*, unsigned int*, unsigned char*, unsigned char*);
void* EventFxMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void* WPuppet_FindOrCacheBone(void*, const char*);
void* WPuppet_FindNamedBoneEntry(void*, const char*);
extern unsigned int s_eventFxMapDefaultValueStorage;

struct BoneEffectPositionSource {
    char prefix[0x78];
    char positionData[12];
};

struct EntryObject {
    char storage[0x3c];
    // Construct an owned effect entry, forwarding its resource/attachment arguments.
    EntryObject(const char* a, const char* b, int c, int d, int e) {
        ((CEventFx*)this)->Init(a, (void*)b, (void*)c, (void*)d, (void*)e);
    }
};

// Delete a mapped effect through its effect interface and clear the stored pointer.
static void clearEntry(TreeNode* node) {
    void* value = ((TreeNode*)node)->value;
    if (value) {
        ((void (**)(void*, int))(*(char**)value + 0x38))[1](value, 1);
        ((TreeNode*)node)->value = 0;
    }
}

extern char s_cameraTransform[];

// Compare byte keys with the operand load order required by the matching tree code.
static inline bool lessEntryKey(const unsigned char& a, const unsigned char& b) {
    unsigned int bv = b;
    unsigned int av = a;
    return av < bv;
}

// Replace the keyed effect using the shared camera transform as its position source.
void EventFxMap_ReplaceWithDefaultSource(Tree* self, unsigned char key, const char* a, unsigned char d);

// Replace the keyed effect using a cached bone position; return if the bone cannot be resolved.
void EventFxMap_SetBoneAttachedEffect(Tree* self, unsigned char key, const char* a, void* data, const char* source,
                                      int extra);

// Replace the keyed effect using the named bone's position and attachment offset.
void EventFxMap_ReplaceFromNamedBone(Tree* self, unsigned char key, const char* a, void* data, const char* source,
                                     int extra);

// Replace the keyed effect using the supplied model source's position block.
void EventFxMap_SetEffect(Tree* self, unsigned char key, const char* a, const char* b, unsigned char d);

extern const char __vt__26CEventSoundSC_SND_SE_PITCH[];

struct EventSoundSePitch : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    unsigned char startPitchPercent;
    short pitchDeltaPercent;
    unsigned int durationFrames;
    EventSoundSePitch(const char* arg);
    // Release any heap-backed name and optionally delete the object (see EventSoundSeReq::Destroy).
    void* Destroy(int deletingFlag);
    // Interpolate named-effect pitch from its initial percent over the scripted duration, then clear its name.
    int UpdateEffectPitch(unsigned int frame);
};

// Decode a pitch ramp, clamp its target delta, and retain the initial pitch and duration.

} // extern "C"

} // namespace ns0

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
namespace ns1 {

extern "C" {

extern const float kFloat_6_2831855;
extern float s_eventBgmSwingFullTurn;
extern const float kFloat_255_StreamVolume;
extern const float kPitchPercentScale_BgmScript;
extern const float kFloat_1_EventFx;
extern const float kEventDurationScaleNumerator_SoundScript;
extern const float kEventDurationScaleDenominator_SoundScript;
extern const float kFloat_25_EventSound;
extern const float kFloat_30_EventSound;
extern void* g_soundManager;
extern unsigned char s_scriptBgmPitchPercent;
extern unsigned char s_eventBgmVolumeStorage;
extern unsigned char s_scriptBgmPitchPercent;
extern void SoundManager_RequestBgmStop(void*, int);
extern float SoundManager_GetVolumeByMode(void*, int);
extern void Vec3_SetAll(void*, float);
extern void SoundManager_RequestBgmChange(void*, const char*, int, int);
extern void SoundManager_ScaleStreamVolume(void*, float);
extern void SoundManager_SetStreamPitch(void*, float);
extern int EventTiming_IsDurationScalingEnabled();
extern const char __vt__25CEventSoundSC_SND_BGM_VOL[];

struct SoundTransitionManagerView {
    char padding0[0x38];
    float fadeSeconds;
};
// Embedded swing interface lives at object +3924: +0x0c resets histories/state.
struct SwingInterfaceVtable {
    void* padding0[3];
    void (*init)(void*);
};
struct SwingVirtualBase {
    char pad[0x3924];
};
struct SwingVirtual : SwingVirtualBase {
    virtual void v08();
    virtual void init();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual int count();
};
struct SwingInitializationView {
    char padding0[0x180];
    int sampleCount;
    char padding184[0xc];
    int mirrorXAxis;
    char padding194[0x300 - 0x194];
    int field300;
    char padding304[0x30C - 0x304];
    float swingAngle;
    char padding310[0x38E8 - 0x310];
    int captureState;
    int captureFrameLimit;
    int capturedSampleCount;
    char padding38F4[0x3908 - 0x38F4];
    int accelerationInRange;
    char padding390C[0x3918 - 0x390C];
    char padding3918[0x3924 - 0x3918];
    SwingInterfaceVtable* vtable;
    char padding3928[1];
};
extern const char __vt__15CPadSwingAccele[];
extern const char __vt__26CEventSoundSC_SND_BGM_STOP[];
extern const char __vt__27CEventSoundSC_SND_BGM_PITCH[];
extern const char __vt__26CEventSoundSC_SND_BGM_PLAY[];
extern const char scriptSoundResourceStrings[];
struct StringTag {
    // Construct the empty allocator tag used by script-name replacement.
    StringTag() {}
};
extern void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, StringTag);

// Report that the swing acceleration implementation is available.

struct SwingHandednessView {
    char padding0[0x190];
    int mirrorXAxis;
};

extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

struct BgmStopScriptData {
    char padding0[2];
    unsigned short first;
    unsigned int second;
};
struct Unk802232Out : CEventConte {
    unsigned short first;
    float second;
    Unk802232Out(BgmStopScriptData* data);
};
struct BgmPitchScriptData {
    char padding0[2];
    unsigned short first;
    unsigned int second;
    short value8;
};
struct Unk802236Out : CEventConte {
    unsigned short first;
    unsigned char byte6;
    char padding7;
    short value8;
    unsigned int valueC;
    Unk802236Out(BgmPitchScriptData* data);
    // Interpolate BGM pitch percent, publish it to the sound manager, and finish at the duration limit.
    int ApplyRamp(unsigned int index);
    // Optionally delete this script-command object; it owns no sound-name storage.
    void* Destroy(int deletingFlag);
};
struct BgmPlayScriptData {
    char padding0[2];
    unsigned short first;
    unsigned int second;
    char padding8[4];
    unsigned int third;
};
struct Unk80222CtorOut {
    void* vtable;
    unsigned short first;
    char padding6[2];
    unsigned int field8;
    unsigned int fieldC;
    void* field10;
    float value14;
};

struct SwingSampleUpdateView;
struct SwingSampleUpdateVtable {
    char padding0[0x54];
    int (*getCount)(SwingSampleUpdateView*);
    void (*draw)(SwingSampleUpdateView*, void*, float);
    void (*post)(SwingSampleUpdateView*);
    void (*finish)(SwingSampleUpdateView*);
    void (*update)(SwingSampleUpdateView*, SwingSampleUpdateView*, void*, int);
};
// Virtual dispatch contract (vtable byte offsets): +54 returns output batch
// count; +58 processes history bias/angle; +5c updates the ready-range flag;
// +60 analyzes a completed capture; +64 resamples raw samples into the batch.
// The C++ declarations below retain the original embedded-vtable placement.
struct SwingSampleUpdateView {
    float samples[32][3];
    int sampleCount;
    char padding184[0x0C];
    int mirrorXAxis;
    float shortHistory[30][3];
    int shortHistoryIndex;
    float field300;
    float field304[2];
    int swingAngle;
    char padding310[0x5A0];
    int longHistoryIndex;
    char padding8B4[0x3070];
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual int getCount();
    virtual void draw(void*, float);
    virtual void post();
    virtual void finish();
    virtual void update(SwingSampleUpdateView*, void*, int);
};
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x100];
extern char s_swingInputManagerDtorRegistration[12];
extern const float kFloat_2_9_PadSwing;
extern const float kFloat_neg2_9_PadSwing;
extern const float kFloat_0_0_SwingAcceleration;
extern const float kFloat_neg1_swingAccelerometer;
extern const float kFloat_30_PadSwing;
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern void* InputManager_GetSelectedPad(void*);
extern void Vector_CopyComponents(void*, void*);

// Decode the BGM-stop start frame and convert fade frames to seconds.

// Decode a BGM pitch ramp, retaining the initial pitch, clamped delta, and duration.

// Convert script frame counts for the active video timing mode, preserving zero and one.
inline unsigned int ScaleScriptFrameCount(unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (n < 2)
            return n;
        n = Runtime_ConvertDoubleToUnsigned(
            (double)(kEventDurationScaleNumerator_SoundScript * (float)n / kEventDurationScaleDenominator_SoundScript));
    }
    return n;
}

// Read the stored string length from either inline or heap string storage.
inline unsigned int GetScriptSoundNameLength(Unk80222CtorOut* self) {
    unsigned int result;
    if ((self->field8 >> 31) == 0)
        result = *(unsigned char*)&self->field8 & 0x7f;
    else
        result = self->fieldC;
    return result;
}

// Replace the owned sound name by copying borrowed text through the string allocator ABI.
inline void ReplaceScriptSoundName(Unk80222CtorOut* self, const char* text, char* tags) {
    unsigned int capacity = GetScriptSoundNameLength(self);
    tags[8] = tags[12];
    ((void (*)(void*, int, unsigned int, const char*, const char*, char*))String_ReplaceRange)(
        &self->field8, 0, capacity, text, text + strlen(text), tags + 8);
}

// Append the resource-string suffix to the stored sound name, copying its bytes.
inline void AppendScriptSoundName(Unk80222CtorOut* self, char* tags) {
    unsigned int capacity;
    const char* text = scriptSoundResourceStrings;
    capacity = GetScriptSoundNameLength(self);
    tags[0] = tags[4];
    ((void (*)(void*, int, unsigned int, const char*, const char*, char*))String_ReplaceRange)(
        &self->field8, capacity, 0, text, text + strlen(text), tags);
}
}
}
struct CEventSoundSC_SND_BGM_PLAY : CEventConte {
    unsigned short first;
    char padding6[2];
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    float value14;
    CEventSoundSC_SND_BGM_PLAY(ns1::BgmPlayScriptData* data);
    // Release any heap-backed name and optionally delete the object. A named member, not a
    // destructor: a real destructor would also tear down the name string.
    void* Destroy(int deletingFlag);
    // At the start frame, request the named BGM with its fade, reset sound baselines, and clear the name.
    int RequestBgmChange(unsigned int index);
};

// Script command that stops the BGM after a fade.
struct CEventSoundSC_SND_BGM_STOP : CEventConte {
    unsigned short startFrame; // 0x4
    char padding6[2];          // 0x6
    float fadeSeconds;         // 0x8
    // At the start frame, apply the fade duration, request a BGM stop, and reset script pitch and volume baselines.
    int RequestBgmStop(unsigned int index);
    // Optionally delete this script-command object; it owns no sound-name storage.
    void* Destroy(int deletingFlag);
};
// Decode BGM playback timing, append the resource extension, and convert fade duration to seconds.

namespace ns1 {
extern "C" {

struct BgmTransitionManagerView {
    char padding0[0x38];
    float fadeSeconds;
};

struct BgmVolumeCommandView {
    void* vtable;
    unsigned short startFrame;
    unsigned char initialVolume;
    char padding7;
    short volumeDelta;
    char paddingA[2];
    unsigned int durationFrames;
};
struct BgmVolumeScriptData {
    char padding0[2];
    unsigned short first;
    unsigned int second;
    unsigned char third;
};
}
}
struct CEventSoundSC_SND_BGM_VOL : CEventConte {
    unsigned short startFrame;   // 0x4
    unsigned char initialVolume; // 0x6
    char pad_07;                 // 0x7
    short volumeDelta;           // 0x8
    char pad_0A[2];              // 0xa
    unsigned int durationFrames; // 0xc
    CEventSoundSC_SND_BGM_VOL(ns1::BgmVolumeScriptData* data);
    // Interpolate BGM volume from the saved initial value, publish it, and finish at the duration limit.
    int UpdateRamp(unsigned int index);
    // Optionally delete this script-command object; it owns no sound-name storage.
    void* Destroy(int deletingFlag);
};
// Decode a BGM-volume ramp using the current scripted volume as its initial value.

namespace ns1 {
extern "C" {

// Initialize this BGM/swing unit's full-turn constant.
void EventBgmSwing_InitFullTurn();

} // extern "C"

} // namespace ns1

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----

extern "C" {

extern const float kFloat_1_SwingCapture;
extern const float kFloat_3_SwingInterpolation;
extern const float kFloat_2_9_PadSwing;
extern const float kFloat_neg2_9_PadSwing;
extern const float kFloat_0_5_SwingAccelerometer;

extern const float kFloat_0_0_SwingAcceleration;
extern void Vec3_SetAll(void* ptr, float value);

struct SwingAngleAndRangeView {
    char padding0[0x30C];
    float swingAngle;
    char padding310[0x3908 - 0x310];
    void* accelerationInRange;
};
}

// Evaluate the Y component of a cubic Bezier curve without changing floating-point operation order.
inline float EvaluateCurve(Vec3f* p0, Vec3f* p1, Vec3f* p2, Vec3f* p3, float t) {
    float u = 1.0f - t;
    return u * (u * (p0->y * u)) + t * (u * (3.0f * p1->y * u)) + t * (t * (3.0f * p2->y * u)) + t * (t * (p3->y * t));
}

extern "C" {

#undef PROCESS_AXIS
// Per-axis expansion keeps the three independent clipped-run lifetimes and exact floating-point schedule.
// A tested inline template changes code/relocations; retain this expansion to
// preserve the original executable while documenting its full caller contract.
// Caller context: i/current/output walk source/destination; last/finalPoint are
// the final index/address; p0..p3 are shared curve scratch. active/start are
// independent per axis. Strictly inside (-2.9,2.9) ends a clipped run; the final
// sample also closes it and is copied as an endpoint. Runs shorter than 3 copy
// unchanged. If the next-next sample clips to the same side, leave the run open
// across the intervening sample. Longer runs derive Bezier endpoint slopes from
// neighboring samples; boundary branches avoid accessing before/after the array.
#define PROCESS_AXIS(member, active, start)                                                                           \
    if ((current->member < 2.9f && current->member > -2.9f) || i == last) {                                           \
        if (!active) {                                                                                                \
            output->member = current->member;                                                                         \
        } else {                                                                                                      \
            output->member = current->member;                                                                         \
            int end = i - 1;                                                                                          \
            int length = end - start + 1;                                                                             \
            if (end < last - 1 && source[end].member >= 2.9f && source[end + 2].member >= 2.9f) {                     \
            } else if (end < last - 1 && source[end].member <= -2.9f && source[end + 2].member <= -2.9f) {            \
            } else if (length < 3) {                                                                                  \
                for (int j = start; j <= end; ++j)                                                                    \
                    destination[j].member = source[j].member;                                                         \
                active = 0;                                                                                           \
            } else {                                                                                                  \
                if (start > 1) {                                                                                      \
                    p0.x = (float)(start - 1);                                                                        \
                    p0.y = source[start - 1].member;                                                                  \
                    p1.x = (float)length * 0.5f + (float)(start - 1);                                                 \
                    p1.y = (float)length * 0.5f * (source[start - 1].member - source[start - 2].member) +             \
                           source[start - 1].member;                                                                  \
                } else if (start == 1) {                                                                              \
                    p0.x = (float)start;                                                                              \
                    p0.y = source[start].member;                                                                      \
                    p1.x = (float)length * 0.5f + (float)start;                                                       \
                    p1.y = (float)length * 0.5f * (source[start].member - source[start - 1].member) +                 \
                           source[start].member;                                                                      \
                } else {                                                                                              \
                    p0.x = 0.0f;                                                                                      \
                    p0.y = source[0].member;                                                                          \
                    p1.x = (float)length * 0.5f;                                                                      \
                    p1.y = (float)length * 0.5f * (source[1].member - source[0].member) + source[0].member;           \
                }                                                                                                     \
                if (end < last - 1) {                                                                                 \
                    p3.x = (float)(end + 1);                                                                          \
                    p3.y = source[end + 1].member;                                                                    \
                    p2.x = (float)(end + 1) - (float)length * 0.5f;                                                   \
                    p2.y = (float)length * 0.5f * (source[end + 1].member - source[end + 2].member) +                 \
                           source[end + 1].member;                                                                    \
                } else if (end == last - 1) {                                                                         \
                    p3.x = (float)end;                                                                                \
                    p3.y = source[end].member;                                                                        \
                    p2.x = (float)end - (float)length * 0.5f;                                                         \
                    p2.y = (float)length * 0.5f * (source[end].member - source[end + 1].member) + source[end].member; \
                } else {                                                                                              \
                    p3.x = (float)last;                                                                               \
                    p3.y = finalPoint->member;                                                                        \
                    p2.x = (float)last - (float)length * 0.5f;                                                        \
                    p2.y = (float)length * 0.5f * (finalPoint[-1].member - finalPoint->member) + finalPoint->member;  \
                }                                                                                                     \
                Vec3f* target = destination + start;                                                                  \
                for (int j = 0; j < length; ++j, ++target) {                                                          \
                    float t = (1.0f / (float)(length + 1)) * (float)(j + 1);                                          \
                    target->member = EvaluateCurve(&p0, &p1, &p2, &p3, t);                                            \
                }                                                                                                     \
                active = 0;                                                                                           \
            }                                                                                                         \
        }                                                                                                             \
    } else if (!active) {                                                                                             \
        active = 1;                                                                                                   \
        start = i;                                                                                                    \
    }

#undef PROCESS_AXIS

} // extern "C"

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
namespace ns3 {

extern "C" {

extern const float kNegativeYHoldSeconds_Swing;
extern const float kFloat_neg0_5_SwingAccelerometer;
extern const float kSteadyMagnitudeUpper_Swing;
extern const float kSteadyMagnitudeLower_Swing;
extern const float kSteadyResetSeconds_Swing;
extern const float kHistoryMagnitudeThreshold_Swing;
extern const float kHistoryBiasSampleDivisor_Swing;
extern const float kNegativeZCorrectionThreshold_Swing;
extern const float kNegativeZCorrectionScale_Swing;
extern const float kFloat_0_001_PadSwing;
extern const float kFloat_3_141592_PadSwing;
extern const float kThreeHalfPi_Swing;
extern const float kFloat_neg1_swingAccelerometer;
extern const float kFloat_neg0_001_PadSwingAcceleration;
extern const float kSwingDirectionYSumUpperBound;
extern const float kFloat_15_SwingAccelerometer;
extern const float kPeakWindowDivisor_Swing;
extern const float kFloat_0_8_SwingAnalysis;
extern const float kFloat_0_5_SwingAccelerometer;
extern float Vec3_Length(void*);
extern double atan(double);
extern float Math_SquareRootApprox(float);
extern double acos(double);
extern double asin(double);

typedef Vec3f Vec3Sample;

// slot54 returns the timing-mode resampling count; it is not capturedSampleCount.
struct Vtable54 {
    char unk[0x54];
    int (*slot54)(void*);
};
typedef Vec3f Vec3;

// Motion interface +60 queries the animation-table index; object interface +c0
// binds that named motion with a borrowed channel at controller +8. The full
// backend types are unresolved; these views establish only the used slots.
struct Vtable60 {
    char unk[0x60];
    void (*slot60)();
};
struct Object44 {
    char unk[0x44];
    Vtable60* vtable;
};
extern char __vt__19CPadSwingCtrlMotion[];
extern char swingMotionNames[];
extern void Vector_CopyComponents(const char*, char*);
extern const float kFloat_0_805E0078;
extern const float kFloat_0_1_SwingMotion;
extern const float kReadyAccelerationMinY_Swing;
extern const float kReadyAccelerationMinX_Swing;
extern const float kReadyAccelerationMaxX_Swing;
extern const float kFloat_0_0_SwingAcceleration;
extern void Vec3_SetAll(Vec3*, float);
extern void Vec3_Lerp(Vec3*, Vec3*, Vec3*, float);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern const float kCaptureZThreshold_Swing;
extern const float kFloat_1_SwingCapture;
extern const float kFloat_0_8_SwingAnalysis;
extern const float kFloat_0_5_SwingAccelerometer;
extern const float kFloat_30_PadSwing;
extern const float kMinimumCaptureFrames_Swing;

struct UpdateVtable {
    char padding0000[0x24];
    void (*slot24)(void*, void*, void*, int);
    char padding0028[0x38];
    void (*slot60)(void*, void*, int);
};

struct UpdateObject {
    Vec3 values[32];
    int count;
    char padding0184[0x300 - 0x184];
    Vec3 currentAcceleration;
    char padding030C[0x8C0 - 0x30C];
    Vec3 samples[0x1ff];
    char padding20B4[0xC];
    Vec3 output[512];
    float maxes[8];
    int peakHistoryIndex;
    int captureZThresholdReached;
    int captureState;
    int captureFrameLimit;
    int capturedSampleCount;
    float captureElapsed;
    char padding38F8[0x14];
    Vec3 captureStartAcceleration;
    Vec3 captureEndAcceleration;
    union {
        UpdateVtable* vtable;
        void** vt;
    };
};

float s_swingMotionFullTurn;
extern const float kFloat_6_2831855;
extern int EventTiming_IsDurationScalingEnabled();

// Initialize this motion-controller unit's full-turn constant.
void SwingMotion_InitFullTurn();

// Cancel forward motion by switching to the return phase; report whether a change occurred.

struct UpdateVirtualBase {
    char pad[0x3924];
};
// Capture dispatch: +24 repairs clipped runs; +60 analyzes the repaired
// capture. Other slots preserve ABI order and are not inferred here.
class UpdateVirtual : public UpdateVirtualBase {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24(void*, void*, int);
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60(void*, int);
};

} // extern "C"

} // namespace ns3

extern "C" {
void EventSoundEffects_InitFullTurn();
void EventBgmSwing_InitFullTurn();
void SwingMotion_InitFullTurn();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046221C", _ctors_u80221980_0, EventSoundEffects_InitFullTurn);
STATIC_INIT_ENTRY(".CRT$XCU8046221C", _ctors_u80221980_1, EventBgmSwing_InitFullTurn);
STATIC_INIT_ENTRY(".CRT$XCU8046221C", _ctors_u80221980_2, SwingMotion_InitFullTurn);
#pragma pop

// Function definitions follow the original address order.

namespace ns0 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns0

// Delete every owned event effect, free the map nodes, and optionally release the object.
void* CEventEffect::Destroy(int dealloc) {
    using namespace ns0;
    char* p = (char*)this;
    if (p) {
        AsCEventEffect(p)->effectVtable = (void*)__vt__12CEventEffect;
        // Phase 1: visit payloads in key order, releasing each effect without
        // changing links. The embedded sentinel is the tree-root word at +4.
        if (*(void**)p) {
            {
                register void* node = AsCEventEffect(p)->firstNode;
                char* end = (char*)p + 4;
                while (node != end) {
                    void* value = ((TreeNode*)node)->value;
                    if (value) {
                        if (value)
                            ((ValueObject*)value)->vtbl->release(value, 1);
                        ((TreeNode*)node)->value = 0;
                    }
                    void* next = ((TreeNode*)node)->right;
                    if (next != 0) {
                        void* cur = next;
                        while (*(void**)cur != 0) {
                            cur = *(void**)cur;
                        }
                        node = cur;
                    } else {
                        void* par;
                        // 0x80221980 unit: walk parents until the left-child link matches.
                        while ((par = reinterpret_cast<void*>((unsigned int)((TreeNode*)node)->parent & ~1u)),
                               node != *reinterpret_cast<void**>(par)) {
                            node = par;
                        }
                        node = par;
                    }
                }
            }
            // Phase 2: free nodes after all payloads are gone, then reset the
            // root and cached first iterator to the empty-tree representation.
            DtorNode* first = (*(DtorNode**)&AsCEventEffect(p)->treeRoot);
            if (first) {
                destroyLevelTwoDtorNode(p, first->a);
                destroyLevelTwoDtorNode(p, first->b);
                ns0::appFreeUni__FPv(first);
            }
            *(void**)p = 0;
            AsCEventEffect(p)->treeRoot = 0;
            AsCEventEffect(p)->firstNode = p + 4;
        }
        // Phase 3: container/base cleanup also covers a count-zero tree. These
        // repeated receiver guards are the original nested destructor guards;
        // they protect one cleanup operation, not six different owned resources.
        if (p && p && p && p && p && p) {
            DtorNode* first = (*(DtorNode**)&AsCEventEffect(p)->treeRoot);
            if (first) {
                destroyLevelTwoDtorNode(p, first->a);
                destroyLevelTwoDtorNode(p, first->b);
                ns0::appFreeUni__FPv(first);
            }
        }
        if (dealloc > 0)
            __dl__FPv(this);
    }
    return this;
}
#pragma pop

namespace ns0 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Replace the keyed effect using the shared camera transform as its position source.
void EventFxMap_ReplaceWithDefaultSource(Tree* self, unsigned char key, const char* a, unsigned char d) {
    if (self->count) {
        TreeNode* node = self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        while (node) {
            if (node->key >= key) {
                candidate = node;
                node = node->left;
            } else
                node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key)
            candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) {
                delete value;
                candidate->value = 0;
            }
        }
    }
    EntryObject* object = new EntryObject(a, s_cameraTransform, 0, d, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct {
            unsigned char k;
            unsigned int v;
        } pr;
        pr.k = key;
        pr.v = s_eventFxMapDefaultValueStorage;
        slot = (char*)EventFxMap_InsertNode(self, found, e, f, &pr) + 0xC;
    }
    *(void**)(slot + 4) = object;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Replace the keyed effect using the supplied model source's position block.
void EventFxMap_SetEffect(Tree* self, unsigned char key, const char* a, const char* b, unsigned char d) {
    if (self->count) {
        TreeNode* node = self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        while (node) {
            if (node->key >= key) {
                candidate = node;
                node = node->left;
            } else
                node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key)
            candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) {
                delete value;
                candidate->value = 0;
            }
        }
    }
    EntryObject* object = new EntryObject(a, b + 0x50, 0, d, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct {
            unsigned char k;
            unsigned int v;
        } pr;
        pr.k = key;
        pr.v = s_eventFxMapDefaultValueStorage;
        slot = (char*)EventFxMap_InsertNode(self, found, e, f, &pr) + 0xC;
    }
    *(void**)(slot + 4) = object;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Replace the keyed effect using the named bone's position and attachment offset.
void EventFxMap_ReplaceFromNamedBone(Tree* self, unsigned char key, const char* a, void* data, const char* source,
                                     int extra) {
    void* descriptor = WPuppet_FindNamedBoneEntry(data, source);
    if (!descriptor)
        return;
    if (self->count) {
        TreeNode* node = *(TreeNode* volatile*)&self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        unsigned int k2;
        unsigned int kr = *(volatile unsigned char*)&key;
        k2 = *(volatile unsigned char*)&key;
        while (node) {
            if (node->key >= k2) {
                candidate = node;
                node = node->left;
            } else
                node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || kr < candidate->key)
            candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) {
                delete value;
                candidate->value = 0;
            }
        }
    }
    EntryObject* object = new EntryObject(a, ((BoneEffectPositionSource*)*(void**)descriptor)->positionData,
                                          (int)((char*)*((void**)descriptor + 1) + 0x18), extra, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct {
            unsigned char k;
            unsigned int v;
        } pr;
        pr.k = key;
        pr.v = s_eventFxMapDefaultValueStorage;
        slot = (char*)EventFxMap_InsertNode(self, found, e, f, (unsigned char*)&pr) + 0xC;
    }
    *(void**)(slot + 4) = object;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Replace the keyed effect using a cached bone position; return if the bone cannot be resolved.
void EventFxMap_SetBoneAttachedEffect(Tree* self, unsigned char key, const char* a, void* data, const char* source,
                                      int extra) {
    void* descriptor = WPuppet_FindOrCacheBone(data, source);
    if (!descriptor)
        return;
    if (self->count) {
        TreeNode* node = *(TreeNode* volatile*)&self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        unsigned int k2;
        unsigned int kr = *(volatile unsigned char*)&key;
        k2 = *(volatile unsigned char*)&key;
        while (node) {
            if (node->key >= k2) {
                candidate = node;
                node = node->left;
            } else
                node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || kr < candidate->key)
            candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) {
                delete value;
                candidate->value = 0;
            }
        }
    }
    EntryObject* object = new EntryObject(a, ((BoneEffectPositionSource*)descriptor)->positionData, 0, extra, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct {
            unsigned char k;
            unsigned int v;
        } pr;
        pr.k = key;
        pr.v = s_eventFxMapDefaultValueStorage;
        slot = (char*)EventFxMap_InsertNode(self, found, e, f, &pr) + 0xC;
    }
    *(void**)(slot + 4) = object;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Delete the effect under this byte key and erase its map node if present.
void EventFxMap_EraseByKey(Tree* self_, unsigned char key) {
    Tree* self = self_;
    TreeNode* candidate;
    TreeNode* erased;
    if (self->count == 0)
        return;
    TreeNode* node = self->treeRoot;
    candidate = (TreeNode*)&self->treeRoot;
    while (node) {
        if (node->key >= key) {
            candidate = node;
            node = node->left;
        } else
            node = node->right;
    }
    if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key)
        candidate = (TreeNode*)&self->treeRoot;
    if (candidate == (TreeNode*)&self->treeRoot)
        return;
    VirtValue* value = (VirtValue*)candidate->value;
    if (value) {
        delete value;
        candidate->value = 0;
    }
    erased = candidate;
    if (erased == self->firstNode) {
        TreeNode* x = candidate->right;
        if (x) {
            while (x->left)
                x = x->left;
            candidate = x;
        } else {
            TreeNode* p;
            // 0x80221980 unit: walk parents until the left-child link matches.
            while ((p = (TreeNode*)((unsigned int)candidate->parent & ~1u)), candidate != p->left) {
                candidate = p;
            }
            candidate = p;
        }
        self->firstNode = candidate;
    }
    RBTree_RebalanceAfterErase(erased, self->treeRoot);
    appFreeUni__FPv(erased);
    --self->count;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Update attached effect positions by adding each bone/source position to its configured offset.
void EventFxMap_UpdateAttachedPositions(CEventEffect* self) {
    void* node = self->firstNode;
    void* r30;
    char* end = (char*)&self->treeRoot;
    while (node != end) {
        r30 = ((TreeNode*)node)->value;
        if ((*(int*)&((CEventFx*)r30)->spray) != 0 && (*(int*)&((CEventFx*)r30)->sequence) != 0 &&
            (*(int*)&((CEventFx*)r30)->positionOffset) != 0) {
            WVector tmp = *(WVector*)(((CEventFxPositionLayout*)r30)->basePosition) +
                          *(WVector*)(reinterpret_cast<const float*>(((CEventFx*)r30)->positionOffset));
            *(WVector*)(((CEventFxPositionLayout*)r30)->position) = tmp;
        }
        void* next = ((TreeNode*)node)->right;
        if (next != 0) {
            while (*reinterpret_cast<void**>(next) != 0) {
                next = *reinterpret_cast<void**>(next);
            }
            node = next;
        } else {
            void* par;
            // 0x80221980 unit: walk parents until the left-child link matches.
            while ((par = reinterpret_cast<void*>((unsigned int)((TreeNode*)node)->parent & ~1u)),
                   node != *reinterpret_cast<void**>(par)) {
                node = par;
            }
            node = par;
        }
    }
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Optionally delete a fade command that owns no additional resources.
void* CEventEffectSC_EFF_FADE::Destroy(int dealloc) {
    using namespace ns0;
    if (this != 0 && dealloc > 0) {
        __dl__FPv(this);
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Initialize this script/effect unit's full-turn constant.
void EventSoundEffects_InitFullTurn() {
    s_eventSoundEffectsFullTurn = kFloat_6_2831855;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Decode the request command's start frame and copy its sound name.
EventSoundSeReq::EventSoundSeReq(const char* arg) {
    *(volatile unsigned int*)&text.flags = 0;
    *(void* volatile*)&this->vtable = (void*)__vt__24CEventSoundSC_SND_SE_REQ;
    *(volatile unsigned int*)&text.length = 0;
    *(volatile char* volatile*)&text.data = 0;
    unsigned int count = *(const volatile unsigned short*)(arg + 2);
    unsigned int v = scaleTargetCount(count);
    this->value = v;
    const char* str = arg + 4;
    AssignSoundCommandName(text, str);
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// At the start frame, request the named sound once, reset pitch, and clear the command's name.
int EventSoundSeReq::PlayNamedSoundOnce(unsigned int frame) {
    if (frame < value) {
        return 0;
    }
    void* soundName;
    unsigned int bit = (unsigned int)text.flags >> 31;
    if (bit == 0) {
        soundName = (char*)&text.flags + 1;
    } else {
        soundName = text.data;
    }
    Sound_PlayByName(g_soundManager, (const char*)soundName, 0, 0, (const char*)0, 0, kFloat_0_5_EventSound,
                     kFloat_200_EventFx, kFloat_1_EventFx);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    bit = (unsigned int)text.flags >> 31;
    ClearSoundCommandName(text, bit);
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Decode the playback command's timing/options and copy its sound name.
EventSoundSePlay::EventSoundSePlay(const char* arg) {
    *(volatile unsigned int*)&text.flags = 0;
    *(void* volatile*)&this->vtable = (void*)__vt__25CEventSoundSC_SND_SE_PLAY;
    *(volatile unsigned int*)&text.length = 0;
    *(volatile char* volatile*)&text.data = 0;
    unsigned int count = *(const volatile unsigned short*)(arg + 2);
    unsigned int v = scaleCtorCount(count);
    this->value = v;
    const char* str = arg + 4;
    AssignSoundCommandName(text, str);
    this->flag = (unsigned char)arg[1] != 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// At the start frame, play the sound with its alias/loop flag, then reset pitch and clear the name.
int EventSoundSePlay::TryPlaySoundEffect(unsigned int frame) {
    if (frame < value) {
        return 0;
    }
    unsigned int inv = !((unsigned int)text.flags >> 31);
    const char* alias;
    if (inv) {
        alias = (const char*)((char*)&text.flags + 1);
    } else {
        alias = (const char*)text.data;
    }
    void* manager = g_soundManager;
    const char* soundName;
    if (inv != 0) {
        soundName = (const char*)((char*)&text.flags + 1);
    } else {
        soundName = (const char*)text.data;
    }
    register unsigned int loopFlag = flag;
    ((void (*)(void*, const char*, int, int, const char*, int, float, float, float))Sound_PlayByName)(
        manager, soundName, 0, loopFlag, alias, 0, kFloat_0_5_EventSound, kFloat_200_EventFx, kFloat_1_EventFx);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    ClearSoundCommandName(text, (unsigned int)text.flags >> 31);
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Decode the stop command's timing and copy the sound name it targets.
EventSoundSeStop::EventSoundSeStop(const char* arg) {
    *(volatile unsigned int*)&text.flags = 0;
    *(void* volatile*)&this->vtable = (void*)__vt__25CEventSoundSC_SND_SE_STOP;
    *(volatile unsigned int*)&text.length = 0;
    *(volatile char* volatile*)&text.data = 0;
    unsigned int count = *(const volatile unsigned short*)(arg + 2);
    unsigned int v = scaleCtorCount(count);
    this->value = v;
    const char* str = arg + 4;
    AssignSoundCommandName(text, str);
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// At the start frame, stop the named sound alias, reset pitch, and clear the command's name.
int EventSoundSeStop::StopSoundAlias(unsigned int frame) {
    if (frame < value) {
        return 0;
    }
    void* soundName;
    unsigned int bit = (unsigned int)text.flags >> 31;
    if (bit == 0) {
        soundName = (char*)&text.flags + 1;
    } else {
        soundName = text.data;
    }
    Sound_StopByAlias(g_soundManager, (const char*)soundName);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    bit = (unsigned int)text.flags >> 31;
    ClearSoundCommandName(text, bit);
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Decode the sound pitch command's timing, pitch delta and owned target name.
EventSoundSePitch::EventSoundSePitch(const char* arg) {
    *(volatile unsigned int*)&text.flags = 0;
    *(void* volatile*)&this->vtable = (void*)__vt__26CEventSoundSC_SND_SE_PITCH;
    *(volatile unsigned int*)&text.length = 0;
    *(volatile char* volatile*)&text.data = 0;
    unsigned int count = *(const volatile unsigned short*)(arg + 2);
    unsigned int v = scaleTargetCount(count);
    const char* str = arg + 0xc;
    this->value = v;
    unsigned int capacity;
    if ((text.flags >> 31) == 0)
        capacity = *(unsigned char*)&text & 0x7f;
    else
        capacity = text.length;
    volatile struct {
        char value;
        char padding[3];
    } unused;
    char local = unused.value;
    unsigned int length = strlen(str);
    String_ReplaceRange(&text, 0, capacity, str, str + length, &local);
    *(volatile unsigned char*)&this->startPitchPercent = s_eventSoundEffectPitchPercent;
    short amount = *(const volatile short*)(arg + 8);
    this->pitchDeltaPercent = amount;
    if (amount > kMaximumPitchDelta)
        this->pitchDeltaPercent = kMaximumPitchDelta;
    if (this->pitchDeltaPercent < kMinimumPitchDelta)
        this->pitchDeltaPercent = kMinimumPitchDelta;
    this->pitchDeltaPercent += 100;
    *(volatile short*)&this->pitchDeltaPercent = this->pitchDeltaPercent - this->startPitchPercent;
    v = *(const volatile unsigned int*)(arg + 4);
    v = scaleTargetCount(v);
    this->durationFrames = v;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Interpolate named-effect pitch from its initial percent over the scripted duration, then clear its name.
int EventSoundSePitch::UpdateEffectPitch(unsigned int frame) {
    float interp;

    if (frame < value)
        return 0;

    unsigned int delta = frame - value;
    if (durationFrames != 0) {
        interp = (float)delta / (float)durationFrames;
    } else {
        interp = kFloat_1_EventFx;
    }
    short start = pitchDeltaPercent;
    unsigned char initialPitchPercent = startPitchPercent; // pitch-command variant at +0x14
    unsigned int result = (int)((float)start * interp + (float)initialPitchPercent);
    s_eventSoundEffectPitchPercent = (unsigned char)result;
    float ratio = (float)(unsigned char)result / 100.0f;
    SoundManager_SetNamedEffectPitch(g_soundManager, ((text.flags >> 31) == 0) ? (char*)&text.flags + 1 : text.data,
                                     ratio);
    if (delta < durationFrames)
        return 0;
    if (!(text.flags >> 31)) {
        (*(signed char*)((char*)&text.flags + 1)) = 0;
        *(signed char*)&text.flags &= (signed char)0x80;
    } else {
        *text.data = 0;
        text.length = 0;
    }
    return 1;
}
#pragma pop

}

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Scale the BGM playback start frame and copy its resource name into owned storage.
CEventSoundSC_SND_BGM_PLAY::CEventSoundSC_SND_BGM_PLAY(ns1::BgmPlayScriptData* data) {
    using namespace ns1;
    Unk80222CtorOut* self = (Unk80222CtorOut*)this;
    self->field8 = 0;
    self->vtable = (void*)__vt__26CEventSoundSC_SND_BGM_PLAY;
    self->fieldC = 0;
    self->field10 = 0;
    unsigned int first = ScaleScriptFrameCount(data->first);
    self->first = first;
    const char* text = (const char*)data + 8;
    char tags[16];
    ReplaceScriptSoundName(self, text, tags);
    AppendScriptSoundName(self, tags);
    unsigned int second = ScaleScriptFrameCount(data->second);
    self->value14 = (float)second;
    if (EventTiming_IsDurationScalingEnabled())
        self->value14 /= kFloat_25_EventSound;
    else
        self->value14 /= kFloat_30_EventSound;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1

// At the start frame, request the named BGM with its fade, reset sound baselines, and clear the name.
int CEventSoundSC_SND_BGM_PLAY::RequestBgmChange(unsigned int index) {
    using namespace ns1;
    if (index < first)
        return 0;
    ((SoundTransitionManagerView*)g_soundManager)->fadeSeconds = value14;
    const char* soundName;
    if (((unsigned int)text.flags >> 31) == 0)
        soundName = (const char*)((char*)&text.flags + 1);
    else
        soundName = text.data;
    SoundManager_RequestBgmChange(g_soundManager, soundName, 0, 1);
    s_scriptBgmPitchPercent = kPitchUnityPercent;
    float volume = SoundManager_GetVolumeByMode(g_soundManager, 0);
    s_eventBgmVolumeStorage = (unsigned char)(kFloat_255_StreamVolume * volume);
    if (((unsigned int)text.flags >> 31) == 0) {
        *((unsigned char*)&text.flags + 1) = 0;
        *(signed char*)&text.flags &= (signed char)0x80;
    } else {
        *(unsigned char*)text.data = 0;
        text.length = 0;
    }
    return 1;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the scaled BGM stop start frame and its fade duration.
Unk802232Out::Unk802232Out(BgmStopScriptData* data) {
    Unk802232Out* self = this;
    self->vtable = (void*)__vt__26CEventSoundSC_SND_BGM_STOP;
    unsigned int first = data->first;
    if (EventTiming_IsDurationScalingEnabled()) {
        first = (first < 2) ? first
                            : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)first /
                                                              kEventDurationScaleDenominator_SoundScript);
    }
    self->first = first;
    unsigned int second = data->second;
    if (EventTiming_IsDurationScalingEnabled()) {
        second = (second < 2)
                     ? second
                     : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)second /
                                                       kEventDurationScaleDenominator_SoundScript);
    }
    self->second = (float)second;
    if (EventTiming_IsDurationScalingEnabled())
        self->second /= kFloat_25_EventSound;
    else
        self->second /= kFloat_30_EventSound;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1

// Apply the fade duration, request a BGM stop, and reset script pitch and volume baselines.
int CEventSoundSC_SND_BGM_STOP::RequestBgmStop(unsigned int index) {
    using namespace ns1;
    if (index < startFrame) {
        return 0;
    }
    ((BgmTransitionManagerView*)g_soundManager)->fadeSeconds = fadeSeconds;
    SoundManager_RequestBgmStop(g_soundManager, 1);
    s_scriptBgmPitchPercent = kPitchUnityPercent;
    float volume = SoundManager_GetVolumeByMode(g_soundManager, 0);
    s_eventBgmVolumeStorage = (unsigned char)(kFloat_255_StreamVolume * volume);
    return 1;
}
#pragma pop

namespace ns1 {}

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the BGM volume ramp's initial byte value, target delta and scaled timing.
CEventSoundSC_SND_BGM_VOL::CEventSoundSC_SND_BGM_VOL(ns1::BgmVolumeScriptData* data) {
    using namespace ns1;
    BgmVolumeCommandView* self = (BgmVolumeCommandView*)this;
    self->vtable = (void*)__vt__25CEventSoundSC_SND_BGM_VOL;
    unsigned int first = ScaleScriptFrameCount(data->first);
    self->startFrame = (unsigned short)first;
    unsigned char clamped = 0xff;
    unsigned char value = s_eventBgmVolumeStorage;
    self->initialVolume = value;
    unsigned char end = data->third;
    if (end < 0xff)
        clamped = end;
    self->volumeDelta = (short)(clamped - value);
    unsigned int second = ScaleScriptFrameCount(data->second);
    self->durationFrames = second;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1

// Interpolate BGM volume from the saved initial value, publish it, and finish at the duration limit.
int CEventSoundSC_SND_BGM_VOL::UpdateRamp(unsigned int index) {
    using namespace ns1;
    if (index < startFrame)
        return 0;
    unsigned int n = index - startFrame;
    float ratio;
    if (durationFrames != 0)
        ratio = (float)n / (float)durationFrames;
    else
        ratio = kFloat_1_EventFx;
    int value = (int)((float)volumeDelta * ratio + (float)initialVolume);
    s_eventBgmVolumeStorage = value;
    unsigned char byte = (unsigned char)value;
    SoundManager_ScaleStreamVolume(g_soundManager, (float)byte / kFloat_255_StreamVolume);
    if (n < durationFrames)
        return 0;
    return 1;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the BGM pitch ramp relative to its current percent value, clamping the target.
Unk802236Out::Unk802236Out(BgmPitchScriptData* data) {
    Unk802236Out* self = this;
    self->vtable = (void*)__vt__27CEventSoundSC_SND_BGM_PITCH;
    unsigned int first = data->first;
    if (EventTiming_IsDurationScalingEnabled()) {
        first = first < 2 ? first
                          : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)first /
                                                            kEventDurationScaleDenominator_SoundScript);
    }
    self->first = first;
    self->byte6 = s_scriptBgmPitchPercent;
    short value = data->value8;
    self->value8 = value;
    if (self->value8 > kMaximumPitchDelta)
        self->value8 = kMaximumPitchDelta;
    if (self->value8 < -100)
        self->value8 = -100;
    self->value8 += 100;
    self->value8 -= self->byte6;
    unsigned int second = data->second;
    if (EventTiming_IsDurationScalingEnabled()) {
        second = second < 2 ? second
                            : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)second /
                                                              kEventDurationScaleDenominator_SoundScript);
    }
    self->valueC = second;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Interpolate BGM pitch percent, publish it to the sound manager, and finish at the duration limit.
int Unk802236Out::ApplyRamp(unsigned int index) {
    if (index < first)
        return 0;
    unsigned int n = index - first;
    float ratio;
    if (valueC != 0)
        ratio = (float)n / (float)valueC;
    else
        ratio = kFloat_1_EventFx;
    int value = (int)((float)value8 * ratio + (float)byte6);
    s_scriptBgmPitchPercent = value;
    unsigned char byte = (unsigned char)value;
    SoundManager_SetStreamPitch(g_soundManager, (float)byte / kPitchPercentScale_BgmScript);
    if (n < valueC)
        return 0;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1

// The sound-command classes here are declared in ns0; their members are defined in that namespace.
namespace ns0 {
// Release any heap-backed name and optionally delete the object.
void* EventSoundSeReq::Destroy(int deletingFlag) {
    if (this != 0) {
        if (&text.flags != 0 && ((unsigned int)text.flags >> 31) != 0) {
            appFreeUni__FPv(text.data);
        }
        if (deletingFlag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Release any heap-backed name and optionally delete the object.
void* EventSoundSePlay::Destroy(int deletingFlag) {
    if (this != 0) {
        if (&text.flags != 0 && ((unsigned int)text.flags >> 31) != 0) {
            appFreeUni__FPv(text.data);
        }
        if (deletingFlag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Release any heap-backed name and optionally delete the object.
void* EventSoundSeStop::Destroy(int deletingFlag) {
    if (this != 0) {
        if (&text.flags != 0 && ((unsigned int)text.flags >> 31) != 0) {
            appFreeUni__FPv(text.data);
        }
        if (deletingFlag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Release any heap-backed name and optionally delete the object.
void* EventSoundSePitch::Destroy(int deletingFlag) {
    if (this != 0) {
        if (&text.flags != 0 && ((unsigned int)text.flags >> 31) != 0) {
            appFreeUni__FPv(text.data);
        }
        if (deletingFlag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}
#pragma pop
} // namespace ns0

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1

// Release any heap-backed name and optionally delete the object.
void* CEventSoundSC_SND_BGM_PLAY::Destroy(int deletingFlag) {
    if (this != 0) {
        if (&text.flags != 0 && ((unsigned int)text.flags >> 31) != 0) {
            ns1::appFreeUni__FPv(text.data);
        }
        if (deletingFlag > 0) {
            ns1::__dl__FPv(this);
        }
    }
    return this;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Optionally delete this script-command object; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_STOP::Destroy(int deletingFlag) {
    if (this != 0 && deletingFlag > 0) {
        ns1::__dl__FPv(this);
    }
    return this;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Optionally delete this script-command object; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_VOL::Destroy(int deletingFlag) {
    if (this != 0 && deletingFlag > 0) {
        ns1::__dl__FPv(this);
    }
    return this;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Optionally delete this script-command object; it owns no sound-name storage.
void* Unk802236Out::Destroy(int deletingFlag) {
    if (this != 0 && deletingFlag > 0) {
        __dl__FPv(this);
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Initialize this BGM/swing unit's full-turn constant.
void EventBgmSwing_InitFullTurn() {
    s_eventBgmSwingFullTurn = kFloat_6_2831855;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Initialize the swing interface and capture buffers, clear endpoint vectors, and run the reset hook.
CPadSwingAccele* CPadSwingAccele::Init() {
    mirrorXAxis = 0;
    swingVtable = (void*)ns1::__vt__15CPadSwingAccele;
    capturedSampleCount = 0;
    Vec3_SetAll((char*)captureStartAcceleration, kFloat_0_0_SwingAcceleration);
    Vec3_SetAll((char*)captureEndAcceleration, kFloat_0_0_SwingAcceleration);
    char* p1 = (char*)capturedSamples;
    char* p2 = ((char*)&correctedSampleX);
    for (int i = 0; i < kCaptureCapacity; ++i) {
        Vec3_SetAll(p1, kFloat_0_0_SwingAcceleration);
        Vec3_SetAll(p2, kFloat_0_0_SwingAcceleration);
        p1 += 0xC;
        p2 += 0xC;
    }
    Vec3_SetAll((char*)currentAcceleration, kFloat_0_0_SwingAcceleration);
    ((ns1::SwingVirtual*)this)->init();
    return this;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Optionally delete the swing object; its sample buffers are embedded in the object.
void* CPadSwingAccele::Destroy(int deletingFlag) {
    if (this != 0 && deletingFlag > 0) {
        ns1::__dl__FPv(this);
    }
    return this;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Reset sample counts, both history rings, the angle, and the cached last resampling vector.
int CPadSwingAccele::ResetSwingState() {
    char* p;
    int i;
    for (i = 0, p = (char*)shortHistory; i < kShortHistorySamples; ++i, p += 0xc) {
        Vec3_SetAll(p, kFloat_0_0_SwingAcceleration);
    }
    float f0 = kFloat_0_0_SwingAcceleration;
    shortHistoryIndex = 0;
    int longIndex;
    char* longCursor;
    longCursor = (char*)longHistory;
    longIndex = 0;
    swingAngle = f0;
    sampleCount = 0;
    while (longIndex < kLongHistorySamples) {
        Vec3_SetAll(longCursor, kFloat_0_0_SwingAcceleration);
        ++longIndex;
        longCursor += 0xc;
    }
    float f = kFloat_0_0_SwingAcceleration;
    char* q = (char*)this + SWING_OFFSET(lastResampledVector) + (longHistoryIndex = 0);
    negativeYHoldTime = f;
    negativeXHoldTime = f;
    steadyAccelerationTime = f;
    captureState = kCaptureIdle;
    captureFrameLimit = 0;
    accelerationInRange = 0;
    Vec3_SetAll(q, f);
    return 1;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
} // namespace ns1
// Read and clamp the selected pad's samples, mirror X if needed, update both histories, and invoke swing analysis.
int CPadSwingAccele::ReadAndUpdateSwingHistory(void* arg, float dt) {
    float* sp;
    int sampleIndex;
    if (((int&)GetInputManagerSlot()) == 0) {
        if (!ns1::s_inputManagerInitialized) {
            ns1::InputManager_Init(&ns1::s_gameInputManager);
            __register_global_object(&ns1::s_gameInputManager, (void*)ns1::InputManager_Dtor,
                                     ns1::s_swingInputManagerDtorRegistration);
            ns1::s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)&ns1::s_gameInputManager;
    }
    register void* manager = ns1::InputManager_GetSelectedPad((void*)((int&)GetInputManagerSlot()));
    if (!manager) {
        sampleCount = 0;
        return 0;
    }
    ((gtl::CInputExDeviceBase*)manager)->GetDeviceResult(&sampleCount);
    // Acquire at most one raw batch; the update hook copies/resamples stack data.
    float rawSamples[kSampleBatchCapacity][3];
    sampleIndex = 0;
    sp = (float*)rawSamples;
    for (; sampleIndex < sampleCount;) {
        if (sampleIndex >= kSampleBatchCapacity) {
            sampleCount = kSampleBatchCapacity;
            break;
        }
        float tmp[10];
        ((gtl::CScreenPointer*)manager)->GetSample(sampleIndex, (gtl::ScreenPointerSample*)tmp);
        ns1::Vector_CopyComponents(sp, tmp);
        sp += 3;
        ++sampleIndex;
    }
    // Resample first, then clamp every output axis and apply handedness to X.
    ((ns1::SwingSampleUpdateView*)this)->update((ns1::SwingSampleUpdateView*)this, rawSamples, sampleCount);
    sampleCount = ((ns1::SwingSampleUpdateView*)this)->getCount();
    float* cp = (float*)this;
    float lower;
    float upper;
    int clampIndex;
    upper = ns1::kFloat_2_9_PadSwing;
    clampIndex = 0;
    lower = ns1::kFloat_neg2_9_PadSwing;
    for (; clampIndex < sampleCount; ++clampIndex) {
        if (cp[0] > upper)
            cp[0] = upper;
        else if (cp[0] < lower)
            cp[0] = lower;
        if (cp[1] > upper)
            cp[1] = upper;
        else if (cp[1] < lower)
            cp[1] = lower;
        if (cp[2] > upper)
            cp[2] = upper;
        else if (cp[2] < lower)
            cp[2] = lower;
        cp += 3;
    }
    if (mirrorXAxis) {
        const float scale = ns1::kFloat_neg1_swingAccelerometer;
        for (int i = 0; i < sampleCount; ++i) {
            samples[i][0] *= scale;
        }
    }
    // Append oldest-to-newest into both rings; indices wrap at their own capacities.
    for (int i = 0; i < sampleCount; ++i) {
        ns1::Vector_CopyComponents(shortHistory[shortHistoryIndex], samples[sampleCount - i - 1]);
        ++shortHistoryIndex;
        if (shortHistoryIndex >= kShortHistorySamples)
            shortHistoryIndex %= kShortHistorySamples;
        ns1::Vector_CopyComponents(longHistory[longHistoryIndex], samples[sampleCount - i - 1]);
        ++longHistoryIndex;
        if (longHistoryIndex >= kLongHistorySamples)
            longHistoryIndex %= kLongHistorySamples;
    }
    // The mean includes zero-initialized history slots during startup. Store X,Z,Y,
    // then run bias/angle processing and the ready-range query in that order.
    float x = kFloat_0_0_SwingAcceleration, y = x, z = x;
    for (int i = 0; i < kShortHistorySamples; ++i) {
        x += shortHistory[i][0];
        y += shortHistory[i][1];
        z += shortHistory[i][2];
    }
    const float divisor = ns1::kFloat_30_PadSwing;
    currentAcceleration[0] = x / divisor;
    currentAcceleration[1] = z / divisor;
    currentAcceleration[2] = y / divisor;
    ((ns1::SwingSampleUpdateView*)this)->draw(arg, dt);
    ((ns1::SwingSampleUpdateView*)this)->post();
    return 1;
}
namespace ns1 {
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Report that the swing acceleration implementation is available.
int CPadSwingAccele::vfunc_3() {
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Set whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele::SetMirrorXAxis(int value) {
    mirrorXAxis = value;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele::IsMirrorXAxis() {
    return mirrorXAxis;
}
#pragma pop

namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#pragma pop
} // namespace ns1

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Toggle X mirroring and return the resulting handedness flag.
int CPadSwingAccele::ToggleMirrorXAxis() {
    if (mirrorXAxis != 0) {
        mirrorXAxis = 0;
    } else {
        mirrorXAxis = 1;
    }
    return mirrorXAxis;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#define PROCESS_AXIS(member, active, start)                                                                           \
    if ((current->member < 2.9f && current->member > -2.9f) || i == last) {                                           \
        if (!active) {                                                                                                \
            output->member = current->member;                                                                         \
        } else {                                                                                                      \
            output->member = current->member;                                                                         \
            int end = i - 1;                                                                                          \
            int length = end - start + 1;                                                                             \
            if (end < last - 1 && source[end].member >= 2.9f && source[end + 2].member >= 2.9f) {                     \
            } else if (end < last - 1 && source[end].member <= -2.9f && source[end + 2].member <= -2.9f) {            \
            } else if (length < 3) {                                                                                  \
                for (int j = start; j <= end; ++j)                                                                    \
                    destination[j].member = source[j].member;                                                         \
                active = 0;                                                                                           \
            } else {                                                                                                  \
                if (start > 1) {                                                                                      \
                    p0.x = (float)(start - 1);                                                                        \
                    p0.y = source[start - 1].member;                                                                  \
                    p1.x = (float)length * 0.5f + (float)(start - 1);                                                 \
                    p1.y = (float)length * 0.5f * (source[start - 1].member - source[start - 2].member) +             \
                           source[start - 1].member;                                                                  \
                } else if (start == 1) {                                                                              \
                    p0.x = (float)start;                                                                              \
                    p0.y = source[start].member;                                                                      \
                    p1.x = (float)length * 0.5f + (float)start;                                                       \
                    p1.y = (float)length * 0.5f * (source[start].member - source[start - 1].member) +                 \
                           source[start].member;                                                                      \
                } else {                                                                                              \
                    p0.x = 0.0f;                                                                                      \
                    p0.y = source[0].member;                                                                          \
                    p1.x = (float)length * 0.5f;                                                                      \
                    p1.y = (float)length * 0.5f * (source[1].member - source[0].member) + source[0].member;           \
                }                                                                                                     \
                if (end < last - 1) {                                                                                 \
                    p3.x = (float)(end + 1);                                                                          \
                    p3.y = source[end + 1].member;                                                                    \
                    p2.x = (float)(end + 1) - (float)length * 0.5f;                                                   \
                    p2.y = (float)length * 0.5f * (source[end + 1].member - source[end + 2].member) +                 \
                           source[end + 1].member;                                                                    \
                } else if (end == last - 1) {                                                                         \
                    p3.x = (float)end;                                                                                \
                    p3.y = source[end].member;                                                                        \
                    p2.x = (float)end - (float)length * 0.5f;                                                         \
                    p2.y = (float)length * 0.5f * (source[end].member - source[end + 1].member) + source[end].member; \
                } else {                                                                                              \
                    p3.x = (float)last;                                                                               \
                    p3.y = finalPoint->member;                                                                        \
                    p2.x = (float)last - (float)length * 0.5f;                                                        \
                    p2.y = (float)length * 0.5f * (finalPoint[-1].member - finalPoint->member) + finalPoint->member;  \
                }                                                                                                     \
                Vec3f* target = destination + start;                                                                  \
                for (int j = 0; j < length; ++j, ++target) {                                                          \
                    float t = (1.0f / (float)(length + 1)) * (float)(j + 1);                                          \
                    target->member = EvaluateCurve(&p0, &p1, &p2, &p3, t);                                            \
                }                                                                                                     \
                active = 0;                                                                                           \
            }                                                                                                         \
        }                                                                                                             \
    } else if (!active) {                                                                                             \
        active = 1;                                                                                                   \
        start = i;                                                                                                    \
    }
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
// Repair clipped acceleration runs on each axis with a cubic Bezier interpolation of surrounding samples.
int CPadSwingAccele::RepairClippedAccelerationRuns(Vec3f* destination, Vec3f* source, int count) {
    int haveX;
    register Vec3f* current;
    register Vec3f* output;
    register Vec3f* finalPoint;
    int i;
    int last;
    int haveY;
    int haveZ;
    int firstX;
    int firstY;
    int firstZ;
    Vec3f p0;
    Vec3f p1;
    Vec3f p2;
    Vec3f p3;
    last = count - 1;
    output = destination;
    current = source;
    finalPoint = source + last;
    haveX = 0;
    haveY = 0;
    haveZ = 0;
    firstX = 0;
    firstY = 0;
    firstZ = 0;
    for (i = 0; i <= last; ++current, ++output, ++i) {
        PROCESS_AXIS(x, haveX, firstX)
        PROCESS_AXIS(y, haveY, firstY)
        PROCESS_AXIS(z, haveZ, firstZ)
    }
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the current processed swing angle.
float CPadSwingAccele::GetSwingAngle() {
    return swingAngle;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the stored acceleration-range flag through the original pointer-sized interface.
void* CPadSwingAccele::GetAccelerationInRange() {
    return (void*)accelerationInRange;
}
#pragma pop

#undef PROCESS_AXIS

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Clear capture/corrected buffers, arm capture, save its frame limit, and reset peak and endpoint data.
int CPadSwingAccele::ArmCapture(void* frameLimit) {
    char* first = (char*)capturedSamples;
    char* second = (char*)&correctedSampleX;
    for (int i = 0; i < kCaptureCapacity; ++i) {
        Vec3_SetAll(first, kFloat_0_0_SwingAcceleration);
        Vec3_SetAll(second, kFloat_0_0_SwingAcceleration);
        first += 0xC;
        second += 0xC;
    }
    register float zero = kFloat_0_0_SwingAcceleration;
    char* arg0;
    captureFrameLimit = frameLimit;
    arg0 = (char*)captureStartAcceleration;
    peakAccelerationX0 = zero;
    peakAccelerationX1 = zero;
    peakAccelerationX2 = zero;
    peakAccelerationX3 = zero;
    peakAccelerationX4 = zero;
    peakAccelerationX5 = zero;
    peakAccelerationX6 = zero;
    peakAccelerationX7 = zero;
    captureState = kCaptureArmed;
    capturedSampleCount = 0;
    peakSampleIndex = 0;
    peakHistoryIndex = 0;
    captureZThresholdReached = 0;
    Vec3_SetAll((void*)arg0, zero);
    Vec3_SetAll((char*)captureEndAcceleration, kFloat_0_0_SwingAcceleration);
    return 1;
}
#pragma pop

namespace ns3 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
} // namespace ns3
// Capture samples until the threshold/time limit completes the swing, then repair and analyze the capture.
// delta is seconds. enabled gates edge detection only; appending and time-based
// completion still run when false. Return SwingCaptureState (not a success flag).
// Requires count valid newest-first values and two writable 512-vector buffers.
// v24/v60 keep their slot-offset names: these are repair/analyze dispatches, in that
// order, on both closing paths; output is owned by self and remains available.
int CPadSwingAccele::UpdateCapture(int enabled, float delta) {
    using namespace ns3;
    // Idle/completed captures do nothing; save the start acceleration once.
    int state = captureState;
    if (state == kCaptureComplete || state == kCaptureIdle)
        return state;
    if (capturedSampleCount == 0)
        ns3::Vector_CopyComponents((const char*)captureStartAcceleration, (char*)currentAcceleration);

    // Scan the newest-first batch backward to append chronological raw samples.
    // Keep one buffer slot available for the threshold-crossing closing sample.
    float zero;
    float threshold = kCaptureZThreshold_Swing;
    zero = ns3::kFloat_0_0_SwingAcceleration;
    Vec3f* values = (Vec3f*)samples;
    Vec3f* current = &values[sampleCount - 1];
    Vec3f* prior = current;
    int index = sampleCount - 1;
    for (; index >= 0; --current, --prior, --index) {
        if (current->z > threshold) {
            int n = capturedSampleCount;
            if (n < kCaptureReservedEndpoint) {
                ns3::Vector_CopyComponents((const char*)capturedSamples[n], (char*)prior);
                capturedSampleCount++;
                continue;
            }
        }
        if (captureState == kCaptureTriggered) {
            ns3::Vector_CopyComponents((const char*)capturedSamples[capturedSampleCount], (char*)&values[index]);
            capturedSampleCount = capturedSampleCount + 1;
            ((UpdateVirtual*)this)->v24((Vec3f*)&correctedSampleX, capturedSamples, capturedSampleCount);
            ((UpdateVirtual*)this)->v60((Vec3f*)&correctedSampleX, capturedSampleCount);
            captureState = kCaptureComplete;
            ns3::Vector_CopyComponents((const char*)captureEndAcceleration, (char*)currentAcceleration);
            break;
        } else {
            capturedSampleCount = 0;
            captureElapsed = zero;
        }
    }

    // Enabled edge detection: maintain eight batch maxima and compare the
    // current minimum after Z crosses its threshold. An already triggered
    // capture takes the original cancellation path and clears its sample count.
    if (enabled) {
        if (captureState != kCaptureTriggered) {
            float max = ns3::kFloat_0_0_SwingAcceleration;
            Vec3f* p = values;
            for (int i = sampleCount; i > 0; --i, ++p) {
                if (p->x > max)
                    max = p->x;
            }
            (&peakAccelerationX0)[peakHistoryIndex] = max;
            peakHistoryIndex++;
            if (peakHistoryIndex >= kPeakHistorySize)
                peakHistoryIndex = 0;
            float min = ns3::kFloat_1_SwingCapture;
            Vec3f* q = values;
            for (int i = 0; i < sampleCount; ++q, ++i) {
                if (q->x < min)
                    min = q->x;
                if (q->z > kCaptureTriggerZ)
                    captureZThresholdReached = 1;
            }
            if (captureZThresholdReached != 0) {
                for (int i = 0; i < kPeakHistorySize; ++i) {
                    if ((&peakAccelerationX0)[i] - min > kCapturePeakToTroughX)
                        captureState = kCaptureTriggered;
                }
            }
        } else {
            captureState = kCaptureComplete;
            capturedSampleCount = 0;
        }
    }
    // Time fallback uses elapsed seconds * 30 to test frame limits; zero limit
    // allows an open-ended capture. A timed end uses a zero endpoint vector.
    if (captureState != kCaptureComplete && capturedSampleCount > 0) {
        captureElapsed += delta;
        if (ns3::kFloat_30_PadSwing * captureElapsed >= kMinimumCaptureFrames_Swing)
            captureState = kCaptureTriggered;
        if ((int)captureFrameLimit == 0)
            return captureState;
        if (ns3::kFloat_30_PadSwing * captureElapsed >= (float)((int)captureFrameLimit + kCaptureTimeoutGraceFrames)) {
            ((UpdateVirtual*)this)->v24((Vec3f*)&correctedSampleX, capturedSamples, capturedSampleCount);
            ((UpdateVirtual*)this)->v60((Vec3f*)&correctedSampleX, capturedSampleCount);
            captureState = kCaptureComplete;
            ns3::Vec3_SetAll((Vec3f*)captureEndAcceleration, ns3::kFloat_0_0_SwingAcceleration);
        }
    }
    return captureState;
}
namespace ns3 {
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Track five-sample Y trough and Z peak until Y recovers beyond the existing recovery ratio.
static inline void FindSwingRecoveryWindow(Vec3Sample* v, int count, int& i, int& recoveryEndpoint, float& zWindowSum,
                                           float& yWindowSum, float& maximumZWindowSum, float& minimumYWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i) {
        zWindowSum += v[i].z;
        yWindowSum += v[i].y;
    }
    maximumZWindowSum = zWindowSum;
    minimumYWindowSum = yWindowSum;
    for (recoveryEndpoint = kPeakWindowSamples; recoveryEndpoint < count; ++recoveryEndpoint) {
        zWindowSum -= v[recoveryEndpoint - kPeakWindowSamples].z;
        zWindowSum += v[recoveryEndpoint].z;
        if (zWindowSum > maximumZWindowSum)
            maximumZWindowSum = zWindowSum;
        yWindowSum -= v[recoveryEndpoint - kPeakWindowSamples].y;
        yWindowSum += v[recoveryEndpoint].y;
        if (yWindowSum < minimumYWindowSum)
            minimumYWindowSum = yWindowSum;
        if (minimumYWindowSum < kFloat_neg0_5_SwingAccelerometer &&
            yWindowSum > kFloat_0_8_SwingAnalysis * minimumYWindowSum)
            break;
    }
}

// Choose the strongest five-sample weighted Y/Z window before recovery.
static inline void SelectSwingCombinedPeak(Vec3Sample* v, int recoveryEndpoint, float yToZPeakRatio, int& i,
                                           int& selectedEndpoint, float& combinedWindowSum,
                                           float& bestCombinedWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i)
        combinedWindowSum += yToZPeakRatio * v[i].y + v[i].z;
    bestCombinedWindowSum = combinedWindowSum;
    for (i = kPeakWindowSamples; i < recoveryEndpoint; ++i) {
        combinedWindowSum -= yToZPeakRatio * v[i - kPeakWindowSamples].y + v[i - kPeakWindowSamples].z;
        combinedWindowSum += yToZPeakRatio * v[i].y + v[i].z;
        if (combinedWindowSum > bestCombinedWindowSum) {
            bestCombinedWindowSum = combinedWindowSum;
            selectedEndpoint = i;
        }
    }
}

// Sum up to forty X/Y samples, adjusting the endpoint so the backward window is valid.
static inline void SumSwingDirectionWindow(Vec3Sample* v, int count, int& i, int& selectedEndpoint,
                                           float& directionXSum, float& directionYSum) {
    if (count < kDirectionWindowSamples) {
        selectedEndpoint = count - 1;
        for (i = 0; i < count; ++i) {
            directionXSum += v[i].x;
            directionYSum += v[i].y;
        }
    } else {
        if (selectedEndpoint < kDirectionWindowSamples)
            selectedEndpoint = kDirectionWindowSamples - 1;
        for (i = 0; i < kDirectionWindowSamples; ++i) {
            directionXSum += v[selectedEndpoint - i].x;
            directionYSum += v[selectedEndpoint - i].y;
        }
    }
}

// Store the mean magnitude of the five samples ending at the final selected endpoint.
static inline void MeasureSwingPeakStrength(CPadSwingAccele* self, Vec3Sample* v, int& i, float& peakMagnitudeSum) {
    for (i = 0; i < kPeakWindowSamples; ++i)
        peakMagnitudeSum += Vec3_Length(&v[self->peakSampleIndex - i]);
    self->peakMeanMagnitude = peakMagnitudeSum / kPeakWindowDivisor_Swing;
}

// Count overlapping Z windows above half the magnitude sum across the selected endpoint.
static inline void MeasureSwingPeakWidth(CPadSwingAccele* self, Vec3Sample* v, int count, float peakMagnitudeSum,
                                         int& i, int& reachedPeakEndpoint, int& sustainedWindowCount,
                                         float& widthZWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i)
        widthZWindowSum += v[i].z;
    reachedPeakEndpoint = 0;
    sustainedWindowCount = 0;
    for (i = kPeakWindowSamples; i < count; ++i) {
        if (i == self->peakSampleIndex)
            reachedPeakEndpoint = 1;
        widthZWindowSum -= v[i - kPeakWindowSamples].z;
        widthZWindowSum += v[i].z;
        if (widthZWindowSum > 0.5f * peakMagnitudeSum) {
            ++sustainedWindowCount;
        } else {
            if (reachedPeakEndpoint != 0)
                break;
            sustainedWindowCount = 0;
        }
    }
    self->peakWidthSamples = (float)sustainedWindowCount;
}

} // namespace ns3

// Analyze borrowed acceleration samples into direction, peak strength and sustained width.
// Requires five samples; returns 0 without outputs for shorter input, otherwise 1.
// End-to-end example: ten vectors (0,-1,1), mirror disabled. Pass 1 reaches
// count=10 with minY=-5,maxZ=5; pass 2 uses ratio=-1 and ties at endpoint 4.
// Pass 3 uses all ten samples and overrides the endpoint to 9: X=0,Y=-10,
// direction=atan(0/-10), numerically zero. Pass 4 yields mean magnitude sqrt(2).
// Pass 5 sees Z sum 5 > 0.5*(5*sqrt(2)), so endpoints 5..9 give width 5.
// The seed window at endpoint 4 is excluded from that width. These are empirical
// sample-window/threshold policies; no physical calibration law is recovered.
int CPadSwingAccele::AnalyzeCapturedSamples(Vec3f* v, int count) {
    if (count < kPeakWindowSamples)
        return 0;

    int i, reachedPeakEndpoint, sustainedWindowCount, recoveryEndpoint;
    float minimumYWindowSum, directionXSum, directionYSum, maximumZWindowSum;
    float zWindowSum = kFloat_0_0_SwingAcceleration;
    float yWindowSum = zWindowSum;
    // Pass 1: track the strongest five-sample Z sum and most negative Y sum.
    // Stop after negative Y recovers above 80% of its minimum, once that minimum
    // crossed -0.5. recoveryEndpoint is excluded by the next window search.
    ns3::FindSwingRecoveryWindow(v, count, i, recoveryEndpoint, zWindowSum, yWindowSum, maximumZWindowSum,
                                 minimumYWindowSum);

    // Pass 2: balance the Y trough against the Z peak, then maximize the combined
    // signal (Y*ratio + Z) over five samples. For minY=-10,maxZ=5 the ratio is
    // -0.5: a negative Y impulse contributes positively. Clamp minY away from zero.
    if (minimumYWindowSum > kFloat_neg0_001_PadSwingAcceleration)
        minimumYWindowSum = kFloat_neg0_001_PadSwingAcceleration;
    float yToZPeakRatio = maximumZWindowSum / minimumYWindowSum;

    int selectedEndpoint = kPeakWindowSamples - 1;
    float bestCombinedWindowSum;
    float combinedWindowSum = kFloat_0_0_SwingAcceleration;
    ns3::SelectSwingCombinedPeak(v, recoveryEndpoint, yToZPeakRatio, i, selectedEndpoint, combinedWindowSum,
                                 bestCombinedWindowSum);

    // Pass 3: direction uses at most 40 samples ending at the selected endpoint.
    // Short captures use ALL samples and move the endpoint to their last sample;
    // long captures clamp it to >=39 so the backwards sum cannot underflow.
    // This override also determines the five samples used for strength below.
    directionXSum = kFloat_0_0_SwingAcceleration;
    directionYSum = directionXSum;
    ns3::SumSwingDirectionWindow(v, count, i, selectedEndpoint, directionXSum, directionYSum);

    // Bound Z before recomputing the direction ratio; constrain the ratio to <=-1.
    // This is a second ratio with different clamps, not the Pass 2 search weight.
    if (directionYSum > kSwingDirectionYSumUpperBound)
        directionYSum = kSwingDirectionYSumUpperBound;
    float directionYWeight = maximumZWindowSum; // This pass converts the peak sum into a ratio.
    if (directionYWeight >= kFloat_15_SwingAccelerometer)
        directionYWeight = kFloat_15_SwingAccelerometer;
    directionYWeight /= minimumYWindowSum;
    if (directionYWeight > kFloat_neg1_swingAccelerometer)
        directionYWeight = kFloat_neg1_swingAccelerometer;
    directionYSum *= -directionYWeight;
    float angle = (float)atan((double)(directionXSum / directionYSum));
    swingDirectionAngle = angle;
    if (mirrorXAxis != 0)
        swingDirectionAngle = angle * kFloat_neg1_swingAccelerometer;
    peakSampleIndex = selectedEndpoint;

    // Pass 4: strength is mean vector magnitude over the five ending samples.
    float peakMagnitudeSum = kFloat_0_0_SwingAcceleration;
    ns3::MeasureSwingPeakStrength(this, v, i, peakMagnitudeSum);

    // Pass 5: count consecutive five-sample Z windows above half that magnitude
    // SUM (not half the mean). Discard runs ending before the selected endpoint;
    // the first below-threshold window at/after it finishes the search. The seed
    // window ending at 4 is never counted or marked as having reached the peak.
    // Width is a count of overlapping window endpoints, not disjoint samples.
    float widthZWindowSum = kFloat_0_0_SwingAcceleration;
    ns3::MeasureSwingPeakWidth(this, v, count, peakMagnitudeSum, i, reachedPeakEndpoint, sustainedWindowCount,
                               widthZWindowSum);

    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return mean vector magnitude around the detected swing peak.
float CPadSwingAccele::GetPeakMeanMagnitude() {
    return peakMeanMagnitude;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the detected peak width measured in samples.
float CPadSwingAccele::GetPeakWidthSamples() {
    return peakWidthSamples;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the handedness-adjusted direction angle found by the capture analysis.
float CPadSwingAccele::vfunc_14() {
    return swingDirectionAngle;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Accumulate absolute corrected acceleration over the requested capture fraction.
int CPadSwingAccele::SumAbsCorrectedSamplePrefix(int* outCount, Vec3f* values, float fraction) {
    Vec3_SetAll(values, kFloat_0_0_SwingAcceleration);
    int limit = capturedSampleCount;
    int count = ((ns1::SwingVirtual*)this)->count();
    if (fraction * (float)count < (float)limit) {

        limit = (int)(fraction * (float)((ns1::SwingVirtual*)this)->count());
    }
    float zero = kFloat_0_0_SwingAcceleration;
    char* self = (char*)this;
    for (int i = 0; i < limit; i++) {
        float value = ((CPadSwingAccele*)self)->correctedSampleX;
        if (value > zero)
            values->x += value;
        else
            values->x -= value;
        value = ((CPadSwingAccele*)self)->correctedSampleY;
        if (value > zero)
            values->z += value;
        else
            values->z -= value;
        value = ((CPadSwingAccele*)self)->correctedSampleZ;
        if (value > zero)
            values->y += value;
        else
            values->y -= value;
        self += sizeof(Vec3f);
    }
    *outCount = limit;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Accumulate absolute corrected acceleration per axis and return at least one as the divisor count.
int CPadSwingAccele::SumAbsoluteCorrectedSamples(float* outX, float* outY, float* outZ, int* outCount) {
    float accX = kFloat_0_0_SwingAcceleration;
    float accY = kFloat_0_0_SwingAcceleration;
    float accZ = kFloat_0_0_SwingAcceleration;

    char* ptr = (char*)this;

    int count = capturedSampleCount;

    for (int i = 0; i < count; i++) {
        float value = ((CPadSwingAccele*)ptr)->correctedSampleX;
        if (value > (kFloat_0_0_SwingAcceleration))
            accX += value;
        else
            accX -= value;
        value = ((CPadSwingAccele*)ptr)->correctedSampleZ;
        if (value > (kFloat_0_0_SwingAcceleration))
            accY += value;
        else
            accY -= value;
        value = ((CPadSwingAccele*)ptr)->correctedSampleY;
        if (value > (kFloat_0_0_SwingAcceleration))
            accZ += value;
        else
            accZ -= value;
        ptr += sizeof(Vec3f);
    }

    *outX = accX;
    *outY = accY;
    *outZ = accZ;
    count = capturedSampleCount;
    if (!count)
        *outCount = 1;
    else
        *outCount = count;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Copy the stored start/end capture vectors to two required output buffers.
int CPadSwingAccele::CopyCaptureAccelerations(const char* first, const char* second) {
    if (first == 0)
        return 0;
    if (second == 0)
        return 0;
    ns3::Vector_CopyComponents(first, (char*)captureStartAcceleration);
    ns3::Vector_CopyComponents(second, (char*)captureEndAcceleration);
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the number of captured swing samples.
unsigned int CPadSwingAccele::GetCapturedSampleCount() {
    return capturedSampleCount;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Return the timing-mode-dependent resampler output sample count.
int CPadSwingAccele::GetResampleOutputSampleCount() {
    if (ns3::EventTiming_IsDurationScalingEnabled() != 0) {
        return 0xc;
    }
    return 0xa;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Remove held/stationary acceleration bias, clamp axes, and derive the current swing angle.
// delta is seconds; enabled gates history-bias removal only. Always returns 1.
// value is a local processed acceleration copy: clamps/correction do not write
// current back. history/head/hold timers and angle are the modified members.
// angle is radians from +Y toward +X in the processed acceleration plane; the
// negative-X/negative-Y branches use Z or the preserved square-root expression.
int CPadSwingAccele::CorrectAccelAndUpdateAngle(int enabled, float delta) {
    Vec3f value;
    float threshold, total;
    ns3::Vector_CopyComponents((const char*)&value, (char*)&currentAcceleration);
    if (enabled) {
        // Suppress recent history X during negative-axis holds, and clear all
        // history X after steady magnitude persists for the reset interval.
        total = ns3::kFloat_0_0_SwingAcceleration;
        if (value.y < total)
            negativeYHoldTime = ns3::kNegativeYHoldSeconds_Swing;
        float zeroY, tY;
        tY = negativeYHoldTime;
        zeroY = ns3::kFloat_0_0_SwingAcceleration;
        if (tY > zeroY) {
            for (int i = 0; i < sampleCount; ++i) {
                int n = (longHistoryIndex + kLastLongHistorySlot - i) % kLongHistorySamples;
                longHistory[n][0] = zeroY;
            }
            negativeYHoldTime -= delta;
        }
        if (value.x < ns3::kFloat_neg0_5_SwingAccelerometer)
            negativeXHoldTime = ns3::kFloat_0_5_SwingAccelerometer;
        float zeroX, tX;
        tX = negativeXHoldTime;
        zeroX = ns3::kFloat_0_0_SwingAcceleration;
        if (tX > zeroX) {
            for (int i = 0; i < sampleCount; ++i) {
                int n = (longHistoryIndex + kLastLongHistorySlot - i) % kLongHistorySamples;
                longHistory[n][0] = zeroX;
            }
            negativeXHoldTime -= delta;
        }
        float length = ns3::Vec3_Length(&value);
        if (length < ns3::kSteadyMagnitudeUpper_Swing && length > ns3::kSteadyMagnitudeLower_Swing) {
            float limit, s0;
            s0 = steadyAccelerationTime;
            limit = ns3::kSteadyResetSeconds_Swing;
            float s = s0 + delta;
            steadyAccelerationTime = s;
            if (s >= limit) {
                float zero = ns3::kFloat_0_0_SwingAcceleration;
                for (int j = 0; j < kLongHistorySamples; ++j)
                    longHistory[j][0] = zero;
                steadyAccelerationTime = ns3::kFloat_0_0_SwingAcceleration;
            }
        } else
            steadyAccelerationTime = ns3::kFloat_0_0_SwingAcceleration;

        // Ring head is the next write: head+20..head+119 excludes the oldest
        // twenty slots. Sum only samples above the magnitude threshold, but
        // divide by the fixed bias divisor, not by the number accepted.
        threshold = ns3::kHistoryMagnitudeThreshold_Swing;
        for (int i = kBiasHistorySkip; i < kLongHistorySamples; ++i) {
            Vec3f* item = (Vec3f*)longHistory[(longHistoryIndex + i) % kLongHistorySamples];
            if (ns3::Vec3_Length(item) > threshold)
                total += longHistory[(longHistoryIndex + i) % kLongHistorySamples][0];
        }
        // This is cross-axis compensation: historical X lowers present Y.
        // Example: accepted X values sum to 10; the current divisor is 100,
        // so Y=0.6 becomes 0.5. A negative sum leaves Y unchanged. Halving the
        // divisor doubles this subtraction, including when few samples pass.
        // No sensor calibration derivation is established here. Tuning D affects
        // both the angle and any subsequent range decision, not the captured data.
        float average = total / ns3::kHistoryBiasSampleDivisor_Swing;
        if (average > ns3::kFloat_0_0_SwingAcceleration)
            value.y -= average;
    }
    // Clamp the local vector to [-1,1], then apply the original negative-Z correction.
    if (value.x > ns3::kFloat_1_SwingCapture)
        value.x = ns3::kFloat_1_SwingCapture;
    else if (value.x < ns3::kFloat_neg1_swingAccelerometer)
        value.x = ns3::kFloat_neg1_swingAccelerometer;
    if (value.y > ns3::kFloat_1_SwingCapture)
        value.y = ns3::kFloat_1_SwingCapture;
    else if (value.y < ns3::kFloat_neg1_swingAccelerometer)
        value.y = ns3::kFloat_neg1_swingAccelerometer;
    if (value.z > ns3::kFloat_1_SwingCapture)
        value.z = ns3::kFloat_1_SwingCapture;
    else if (value.z < ns3::kFloat_neg1_swingAccelerometer)
        value.z = ns3::kFloat_neg1_swingAccelerometer;
    // Current threshold/scale are -0.8 and 1.5: at Z=-1 this ADDS 0.3 to X.
    // There is no second clamp: X=0.9 can become 1.2 before angle projection.
    if (value.z < ns3::kNegativeZCorrectionThreshold_Swing)
        value.x = value.x - ns3::kNegativeZCorrectionScale_Swing * (ns3::kFloat_0_8_SwingAnalysis + value.z);

    // Piecewise angle in radians, measured from +Y toward +X in the first branch.
    // For X>0: acos(Y/hypot(X,Y)); e.g. (1,0,0) -> pi/2. For X<=0,Y<=0,
    // compare -X with Z to choose the XY or YZ projection. The XY branch uses
    // pi+sqrt(-X/hypot(X,Y)): (-1,0,0) -> pi+1, NOT 3*pi/2. The YZ branch
    // uses pi+asin(Z/hypot(Y,Z)): (0,-1,0) -> pi. Remaining X<=0,Y>0 uses
    // 3*pi/2+asin(Y/hypot(X,Y)): (0,1,0) -> 2*pi. These examples use the
    // already corrected/clamped vector. The unusual sqrt is empirical code,
    // not an established circular-angle identity; replacing it changes behavior.
    // A projection shorter than 0.001 preserves the previous angle.
    if (value.x > ns3::kFloat_0_0_SwingAcceleration) {
        float length = ns3::Math_SquareRootApprox(value.x * value.x + value.y * value.y);
        if (!(length < ns3::kFloat_0_001_PadSwing))
            swingAngle = ns3::acos(value.y / length);
    } else if (value.x <= ns3::kFloat_0_0_SwingAcceleration && value.y <= ns3::kFloat_0_0_SwingAcceleration) {
        if (-value.x > value.z) {
            float length = ns3::Math_SquareRootApprox(value.x * value.x + value.y * value.y);
            if (!(length < ns3::kFloat_0_001_PadSwing))
                swingAngle = ns3::kFloat_3_141592_PadSwing + ns3::Math_SquareRootApprox(-value.x / length);
        } else {
            float length = ns3::Math_SquareRootApprox(value.y * value.y + value.z * value.z);
            if (!(length < ns3::kFloat_0_001_PadSwing))
                swingAngle = ns3::kFloat_3_141592_PadSwing + (float)ns3::asin(value.z / length);
        }
    } else {
        float length = ns3::Math_SquareRootApprox(value.x * value.x + value.y * value.y);
        if (!(length < ns3::kFloat_0_001_PadSwing))
            swingAngle = ns3::kThreeHalfPi_Swing + (float)ns3::asin(value.y / length);
    }
    return 1;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Test current acceleration against the configured X/Y range and store the result.
int CPadSwingAccele::UpdateAccelerationInRangeFlag() {
    char buffer[8];
    ns3::Vector_CopyComponents((const char*)buffer, (char*)currentAcceleration);
    float _limit = ns3::kReadyAccelerationMinY_Swing;
    accelerationInRange = 0;
    float y = *(float*)(buffer + 4);
    if (y >= _limit) {
        float x = *(float*)buffer;
        if (x >= ns3::kReadyAccelerationMinX_Swing && x <= ns3::kReadyAccelerationMaxX_Swing)
            accelerationInRange = 1;
    }
    return accelerationInRange;
}
#pragma pop

namespace ns3 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Query the timing-mode output count at +0x54; its value must stay stable throughout resampling.
static inline int ResampleOutputSampleCount(void* self) {
    return ((Vtable54*)AsCPadSwingAccele(self)->swingVtable)->slot54(self);
}

// Fill the full timing-mode output with the previous vector when there are no raw samples.
static inline void FillResampleOutputFromCache(void* self, Vec3*& out, int& i) {
    for (i = 0; i < ResampleOutputSampleCount(self); ++i) {
        Vector_CopyComponents((const char*)out, (char*)self + SWING_OFFSET(lastResampledVector));
        out++;
    }
}

// Append the cached endpoint in writable source storage; full/oversized batches reuse slot 31.
static inline void AppendResampleCachedEndpoint(void* self, Vec3* values, int& inputSampleCount) {
    if (inputSampleCount < kSampleBatchCapacity) {
        Vector_CopyComponents((const char*)&values[inputSampleCount], (char*)self + SWING_OFFSET(lastResampledVector));
    } else {
        Vector_CopyComponents((const char*)((char*)values + (kSampleBatchCapacity - 1) * sizeof(Vec3)),
                              (char*)self + SWING_OFFSET(lastResampledVector));
        inputSampleCount = kSampleBatchCapacity - 1;
    }
}

// Interpolate newest-first intervals without caching the virtual output count or changing float order.
static inline void InterpolateResampleOutput(void* self, Vec3* values, float t, Vec3*& out, int& outIndex, int& lower) {
    for (outIndex = 0; outIndex < ResampleOutputSampleCount(self); ++outIndex) {
        lower = (int)((float)outIndex / t);
        Vec3_Lerp(out, &values[lower], &values[lower + 1], ((float)outIndex - (float)lower * t) / t);
        out++;
    }
}
}
#pragma pop

} // namespace ns3

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Resample contract (caller owns output and writable newest-first values):
// - inputSampleCount must be >=0. Negative input is not rejected and would write
//   before values; it is outside the contract, not an empty-input convention.
// - Let M be the +0x54 virtual output count; it must stay stable during this call.
//   If M<=0, return 0 without touching arrays. Otherwise output must fit M Vec3s
//   and must not overlap values. Normal timing implementations choose M=10 or 12.
// - N=0 repeats the cached vector M times and returns 0; values/cache are untouched.
// - 1<=N<32 needs N+1 writable source slots: slot N receives the cached older endpoint.
// - N>=32, including oversized reported counts, requires 32 source slots; slot 31
//   is replaced by the cache and N becomes 31. No error reports discarded samples.
// - Nonempty output interpolates lower=floor(j*N/M) and lower+1 for j=0..M-1,
//   then caches values[0] and returns 1. The appended endpoint is not an output slot.
// Example M=10,N=2, values X=[2,1], cached X=0 -> output X at j=0/5/9 is 2/1/0.2;
// source slot 2 becomes 0 and next cache becomes 2. Counts are samples, not frames.
// Resample one batch under the declaration's capacity/count contract, retaining the next endpoint.
int CPadSwingAccele::ResampleNewestFirstBatch(Vec3f* output, Vec3f* values, int inputSampleCount) {
    if (ns3::ResampleOutputSampleCount(this) <= 0)
        return 0;

    if (inputSampleCount == 0) {
        // No input: repeat the previous endpoint, leaving cache and source intact.
        Vec3f* out = output;
        int i;
        ns3::FillResampleOutputFromCache(this, out, i);
        return 0;
    }

    // Append an older endpoint; a full raw batch sacrifices its last sample.
    ns3::AppendResampleCachedEndpoint(this, values, inputSampleCount);

    // Ratio is output samples per input interval, not seconds per sample.
    float t = (float)ns3::ResampleOutputSampleCount(this) / (float)inputSampleCount;
    Vec3f* out = output;
    int outIndex, lower;
    ns3::InterpolateResampleOutput(this, values, t, out, outIndex, lower);
    ns3::Vector_CopyComponents((char*)this + SWING_OFFSET(lastResampledVector), (char*)values);
    return 1;
}
#pragma pop

namespace ns3 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Initialize this motion-controller unit's full-turn constant.
void SwingMotion_InitFullTurn() {
    s_swingMotionFullTurn = kFloat_6_2831855;
}
}
#pragma pop

} // namespace ns3

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Initialize the motion state, timing, interface, and five owned channel pointers.
void CPadSwingCtrlMotion::Init() {
    float zero = kFloat_0_805E0078;
    motionPhase = 0;
    motionVtable = (void*)__vt__19CPadSwingCtrlMotion;
    motionPosition = zero;
    motionVelocity = zero;
    motionAcceleration = zero;
    motionStep = 0.1f;
    motionSettled = 0;
    animationStarted = 0;
    animationElapsed = zero;
    settlingElapsed = zero;
    completionElapsed = zero;
    motionLimit = zero;
    firstOwnedMotionChannel = 0;
    ownedChannel1 = 0;
    ownedChannel2 = 0;
    ownedChannel3 = 0;
    ownedChannel4 = 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Delete all five owned motion-channel arrays and optionally release the controller.
void* CPadSwingCtrlMotion::Destroy(int deletingFlag) {
    if (this != 0) {
        motionVtable = (void*)__vt__19CPadSwingCtrlMotion;
        // Walk the five channel slots by byte offset from the object, as the original does.
        char* ptr = (char*)this;
        int i = 0;
        for (; i < 5; i++) {
            if (((CPadSwingCtrlMotion*)ptr)->firstOwnedMotionChannel != 0) {
                __dla__FPv(((CPadSwingCtrlMotion*)ptr)->firstOwnedMotionChannel);
                ((CPadSwingCtrlMotion*)ptr)->firstOwnedMotionChannel = 0;
            }
            ptr += 4;
        }
        if (deletingFlag > 0)
            ns0::__dl__FPv(this);
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Return the current motion-state value.
unsigned int CPadSwingCtrlMotion::vfunc_1() {
    return motionPhase;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Advance the motion phase, clearing the timing and position data needed by its next phase.
int CPadSwingCtrlMotion::AdvanceMotionPhase() {
    int changed = 0;
    switch (motionPhase) {
    // Start a return cycle with zero position, velocity and acceleration.
    case kMotionIdle: {
        float zero = kFloat_0_805E0078;
        motionPhase = kMotionReturning;
        changed = 1;
        motionPosition = zero;
        motionVelocity = zero;
        motionAcceleration = zero;
        break;
    }
    // Advance the return phase to forward motion.
    case kMotionReturning:
        motionPhase = kMotionForward;
        changed = 1;
        break;
    // Reset animation/settling timers before settling.
    case kMotionForward: {
        float zero = kFloat_0_805E0078;
        motionSettled = 0;
        changed = 1;
        animationStarted = 0;
        animationElapsed = zero;
        settlingElapsed = zero;
        motionPhase = kMotionSettling;
        break;
    }
    // Mark completion and clear completion timing/position.
    case kMotionSettling: {
        float zero = kFloat_0_805E0078;
        motionPhase = kMotionComplete;
        changed = 1;
        motionPosition = zero;
        completionElapsed = zero;
        break;
    }
    // Return to idle; no new motion-change flag.
    case kMotionComplete:
        motionPhase = kMotionIdle;
        break;
    }
    return changed;
}
#pragma pop

namespace ns3 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Cancel forward motion by switching to the return phase; report whether a change occurred.
int CPadSwingCtrlMotion::CancelForwardMotion() {
    int* self = (int*)this;
    int r4 = 0;
    switch (*self) {
    case kMotionIdle:
    case kMotionReturning:
    case kMotionSettling:
        break;
    case kMotionForward:
        *self = kMotionReturning;
        r4 = 1;
        break;
    default:
        break;
    }
    return r4;
}
}
#pragma pop

} // namespace ns3

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Return the motion controller to idle and clear its timing, position, and completion data.
int CPadSwingCtrlMotion::ResetMotionToIdle() {
    float f0 = kFloat_0_805E0078;
    motionPhase = kMotionIdle;
    motionPosition = f0;
    motionVelocity = f0;
    motionAcceleration = f0;
    motionSettled = 0;
    animationStarted = 0;
    animationElapsed = f0;
    settlingElapsed = f0;
    completionElapsed = f0;
    animationStartWaitElapsed = f0;
    return 1;
}
#pragma pop

struct VtableC0 {
    char unk[0xC0];
    void (*slotC0)(void*, void*, const char*);
};
struct Object0 {
    VtableC0* vtable;
};

struct P44B {
    char pad[68];
};
class P44 : public P44B {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual int v60();
};
class PC0 {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60();
    virtual void v64();
    virtual void v68();
    virtual void v6C();
    virtual void v70();
    virtual void v74();
    virtual void v78();
    virtual void v7C();
    virtual void v80();
    virtual void v84();
    virtual void v88();
    virtual void v8C();
    virtual void v90();
    virtual void v94();
    virtual void v98();
    virtual void v9C();
    virtual void vA0();
    virtual void vA4();
    virtual void vA8();
    virtual void vAC();
    virtual void vB0();
    virtual void vB4();
    virtual void vB8();
    virtual void vBC();
    virtual void vC0(char*, const char*);
};

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
// Bind the motion entry selected by the virtual motion-index query to the supplied object.
float CPadSwingCtrlMotion::BindSelectedMotion(Object0* object) {
    ((PC0*)object)->vC0((char*)this + 8, swingMotionNames + ((P44*)this)->v60() * 0x24);
    return kFloat_0_805E0078;
}
#pragma pop
