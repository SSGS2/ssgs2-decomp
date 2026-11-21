#include <decomp/static_init.h>
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
static inline CEventEffect* AsCEventEffect(void* p) { return (CEventEffect*)p; }

// View the pitch-command layout without transferring ownership.
static inline CEventSoundSC_SND_SE_PITCH* AsCEventSoundSC_SND_SE_PITCH(void* p) { return (CEventSoundSC_SND_SE_PITCH*)p; }

// View common script-sound storage at the supplied base address.
static inline CEventSoundSC* AsCEventSoundSC(void* p) { return (CEventSoundSC*)p; }

// View the motion-controller fields without changing the embedded interface offset.
static inline CPadSwingCtrlMotion* AsCPadSwingCtrlMotion(void* p) { return (CPadSwingCtrlMotion*)p; }

// View the shared swing storage underlying this unit's phase-specific layouts.
static inline CPadSwingAccele* AsCPadSwingAccele(void* p) { return (CPadSwingAccele*)p; }

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
#define SWING_OFFSET(field) ((int)&((CPadSwingAccele*)0)->field)

enum { kSampleBatchCapacity = 32, kShortHistorySamples = 30, kLongHistorySamples = 120, kCaptureCapacity = 512, kPeakHistorySize = 8 };
enum { kSwingVectorBytes = 12, kPeakWindowSamples = 5,
       kDirectionWindowSamples = 40, kBiasHistorySkip = 20,
       kLongHistoryOffset = 0x310, kCaptureReservedEndpoint = kCaptureCapacity - 1 };
enum { kCaptureTimeoutGraceFrames = 2, kLastLongHistorySlot = kLongHistorySamples - 1 };
static const float kCaptureTriggerZ = 0.8f;       // Raw device acceleration units.
static const float kCapturePeakToTroughX = 0.5f;  // Raw device acceleration difference.
// Buffer/time model: packed XYZ float triplets have 12-byte stride. A pad batch
// arrives newest first (up to 32); vfunc_23 resamples it to the timing-mode count
// (10 or 12), preserving a previous-batch endpoint. Both history rings append in
// reverse batch order, oldest to newest; their index points to the NEXT write.
// shortHistory holds 30 samples, longHistory 120. They are sample windows, not
// fixed durations without the pad/update rate. currentAcceleration at +300 stores
// the short-window mean in X,Z,Y order. Raw acceleration units are device-defined;
// clamping is +/-2.9. Capture stores chronological samples in 512-vector buffers,
// with slot 511 reserved for a closing endpoint. Capture elapsed time is converted
// to 30-Hz frame units for its limit tests; sample counts and frames differ.
enum SwingCaptureState { kCaptureIdle = 0, kCaptureArmed = 1, kCaptureTriggered = 2, kCaptureComplete = 3 };
enum SwingMotionState { kMotionIdle = 0, kMotionReturning = 1, kMotionForward = 2, kMotionSettling = 3, kMotionComplete = 4 };
// Two independent state domains; do not compare their numeric values to each other.
// Capture: reset -> Idle; start(vfunc_10) -> Armed; update(vfunc_11) -> Triggered
// on the Z/X edge or elapsedSeconds*30>=2. A closing sample runs repair then
// analysis and enters Complete. A nonzero frame limit also closes at limit+2;
// zero disables that timeout. An enabled update already Triggered can cancel to
// Complete with count=0. Idle/Complete updates return without advancing storage.
// Motion: vfunc_2 advances Idle->Returning->Forward->Settling->Complete->Idle;
// vfunc_3 cancels Forward->Returning, and vfunc_4 resets to Idle. Those methods
// clear their own motion timers, not captureElapsed or captureFrameLimit.
// No capture->motion transition call is present in this unit. The owner drives
// both interfaces; changing capture timeout does not itself advance motion.

enum { kPitchUnityPercent = 100, kMinimumPitchDelta = -100, kMaximumPitchDelta = 9900 };

namespace ns0 {
// Translation unit 0x80221980-0x80222FB0 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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

struct DtorNode { DtorNode* a; DtorNode* b; };
struct ValueVtbl { char pad[8]; void (*release)(void*, int); };
struct ValueObject { char pad[0x38]; ValueVtbl* vtbl; };
// Release both child subtrees and the node storage.
static void destroyLeafDtorNode(void* owner, DtorNode* node) {
    if (!node) return;
    if (node->a) ScriptByteMap_FreeSubtree(owner, node->a);
    if (node->b) ScriptByteMap_FreeSubtree(owner, node->b);
    appFreeUni__FPv(node);
}
// Expand one level of subtree cleanup while retaining the original compiler inlining depth.
static void destroyLevelOneDtorNode(void* owner, DtorNode* node) {
    if (!node) return;
    destroyLeafDtorNode(owner, node->a);
    destroyLeafDtorNode(owner, node->b);
    appFreeUni__FPv(node);
}
// Expand two levels of subtree cleanup while retaining the original compiler inlining depth.
static void destroyLevelTwoDtorNode(void* owner, DtorNode* node) {
    if (!node) return;
    destroyLevelOneDtorNode(owner, node->a);
    destroyLevelOneDtorNode(owner, node->b);
    appFreeUni__FPv(node);
}

#pragma use_lmw_stmw on
// Delete every owned event effect, free the map nodes, and optionally release the manager.
void* CEventEffect_dtor(void* self, int dealloc);

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
        if (count < 2) return count;
        return Runtime_ConvertDoubleToUnsigned((kEventDurationScaleNumerator_SoundScript * (float)count) / kEventDurationScaleDenominator_SoundScript);
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
    if ((text.flags >> 31) == 0) existingLength = *(unsigned char*)&text & 0x7f;
    else existingLength = text.length;
    volatile struct { char value; char padding[3]; } allocatorStorage;
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
};

// Decode the request start frame and copy its sound name.

// Convert script frame counts for the active video timing mode; preserve counts zero and one.
static inline unsigned int scaleCtorCount(unsigned int count) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (count < 2) return count;
        return Runtime_ConvertDoubleToUnsigned((kEventDurationScaleNumerator_SoundScript * (float)count) / kEventDurationScaleDenominator_SoundScript);
    }
    return count;
}

struct EventSoundSeStop : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    EventSoundSeStop(const char* arg);
};

// Decode the stop start frame and copy its sound alias.


struct EventSoundSePlay : CEventConte {
    unsigned short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    unsigned int flag;
    EventSoundSePlay(const char* arg);
};

// Decode the play start frame, sound name, and loop flag.


extern const float kFloat_6_2831855;
extern float s_eventSoundEffectsFullTurn;

// Initialize this script/effect unit's full-turn constant.
void EventSoundEffects_InitFullTurn();

void* __dl__FPv(void*);

// Optionally delete a fade command that owns no additional resources.
void* CEventEffectSC_EFF_FADE_dtor(void* self, int dealloc);

extern void* g_soundManager;
extern char s_eventSoundEffectPitchPercent;
extern const float kFloat_0_5_EventSound;
extern const float kFloat_200_EventFx;
extern const float kFloat_1_EventFx;
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void Sound_StopByAlias(void*, const char*);
void __pl__FRC7WVectorRC7WVector(float*, const float*, const float*);
void __as__7WVectorFRC7WVector(float*, const float*);

struct ScriptSoundCommandView {
    char padding0[4];
    unsigned short startFrame;
    int soundStorageFlags;
    int soundNameLength;
    void* soundNameData;
    int loopFlag;
};

// Clear a sound name while keeping its inline/heap allocation for later destruction.
// heapStorage is the freshly read high storage bit after the sound-manager call.
static inline void ClearSoundCommandName(ScriptSoundCommandView* self, unsigned int heapStorage) {
    if (heapStorage == 0) {
        char* stringBytes = (char*)&self->soundStorageFlags;
        stringBytes[1] = 0;
        int storageAndLength = stringBytes[0];
        storageAndLength &= ~0x7F;
        stringBytes[0] = (char)storageAndLength;
    } else {
        char* stringBytes = (char*)self->soundNameData;
        stringBytes[0] = 0;
        self->soundNameLength = 0;
    }
}

// At the start frame, stop the named sound alias, reset pitch, and clear the command's name.
int CEventSoundSC_SND_SE_STOP_StopSoundAlias(ScriptSoundCommandView* self, unsigned int frame);

// At the start frame, request the named sound once, reset pitch, and clear the command's name.
int CEventSoundSC_SND_SE_REQ_PlayNamedSoundOnce(ScriptSoundCommandView* self, unsigned int frame);

// At the start frame, play the sound with its alias/loop flag, then reset pitch and clear the name.
int CEventSoundSC_SND_SE_PLAY_TryPlaySoundEffect(ScriptSoundCommandView* self, unsigned int frame);

// Update attached effect positions by adding each bone/source position to its configured offset.
void EventFxMap_UpdateAttachedPositions(CEventEffect* self);

// treeRoot (+4) is the search root; its address doubles as the end sentinel.
// firstNode (+8) caches the first iterator and advances to treeNext on erase.
struct Tree { unsigned int count; TreeNode* treeRoot; TreeNode* firstNode; };
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
        while (x->left) x = x->left;
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
    if (it == self->firstNode) { it = treeNext(it); self->firstNode = it; }
    RBTree_RebalanceAfterErase(erased, self->treeRoot);
    appFreeUni__FPv(erased);
    --self->count;
}

#pragma opt_lifetimes off
// Delete the effect under this byte key and erase its map node if present.
void EventFxMap_EraseByKey(Tree* self_, unsigned char key);
#pragma opt_lifetimes reset

void SoundManager_SetNamedEffectPitch(void*, const char*, float);
// Interpolate named-effect pitch from its initial percent over the scripted duration, then clear its name.
int CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch(void* self, unsigned int value);

void* __nw__FUl(unsigned int);
void CEventFx_Ctor(void*, const char*, const char*, int, int, int);
void* EventFxMap_FindInsertionPosition(void*, unsigned char*, unsigned int*, unsigned char*, unsigned char*);
void* EventFxMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void* WPuppet_FindOrCacheBone(void*, const char*);
void* WPuppet_FindNamedBoneEntry(void*, const char*);
extern unsigned int s_eventFxMapDefaultValueStorage;

struct BoneEffectPositionSource { char prefix[0x78]; char positionData[12]; };

struct EntryObject {
    char storage[0x3c];
    // Construct an owned effect entry, forwarding its resource/attachment arguments.
    EntryObject(const char* a, const char* b, int c, int d, int e) {
        CEventFx_Ctor(this, a, b, c, d, e);
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
void EventFxMap_SetBoneAttachedEffect(Tree* self, unsigned char key, const char* a, void* data, const char* source, int extra);

// Replace the keyed effect using the named bone's position and attachment offset.
void EventFxMap_ReplaceFromNamedBone(Tree* self, unsigned char key, const char* a, void* data, const char* source, int extra);

// Replace the keyed effect using the supplied model source's position block.
void EventFxMap_SetEffect(Tree* self, unsigned char key, const char* a, const char* b, unsigned char d);

extern const char __vt__26CEventSoundSC_SND_SE_PITCH[];

struct EventSoundSePitch : CEventConte {
    short value;
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    unsigned char startPitchPercent;
    short pitchDeltaPercent;
    unsigned int durationFrames;
    EventSoundSePitch(const char* arg);
};

// Decode a pitch ramp, clamp its target delta, and retain the initial pitch and duration.


} // extern "C"

} // namespace ns0

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
namespace ns1 {
// Translation unit 0x80222FB0-0x80224150 (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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

// BGM play command: base/start frame, owned packed sound name, then fade seconds.
// The address-derived type name is retained; no additional fields are inferred.
struct Unk80222Init {
    char padding0[4]; unsigned short startFrame; char padding6[2];
    unsigned int soundStorageFlags; unsigned int soundNameLength; void* soundNameData; float fadeSeconds;
};
struct SoundTransitionManagerView { char padding0[0x38]; float fadeSeconds; };
// Embedded swing interface lives at object +3924: +0x0c resets histories/state.
struct SwingInterfaceVtable { void* padding0[3]; void (*init)(void*); };
struct SwingVirtualBase { char pad[0x3924]; };
struct SwingVirtual : SwingVirtualBase {
    virtual void v08(); virtual void init(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34();
    virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4C(); virtual void v50(); virtual int count();
};
struct SwingInitializationView {
    char padding0[0x180]; int sampleCount; char padding184[0xc]; int mirrorXAxis;
    char padding194[0x300-0x194]; int field300;
    char padding304[0x30C-0x304]; float swingAngle;
    char padding310[0x38E8-0x310]; int captureState; int captureFrameLimit; int capturedSampleCount;
    char padding38F4[0x3908-0x38F4]; int accelerationInRange;
    char padding390C[0x3918-0x390C];
    char padding3918[0x3924-0x3918]; SwingInterfaceVtable* vtable;
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
int CPadSwingAccele_vfunc_3();

struct SwingHandednessView {
    char padding0[0x190];
    int mirrorXAxis;
};

// Return whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele_vfunc_5(SwingHandednessView* self);

// Set whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele_SetMirrorXAxis(SwingHandednessView* self, int value);

// Toggle X mirroring and return the resulting handedness flag.
int CPadSwingAccele_ToggleMirrorXAxis(SwingHandednessView* self);

extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

struct ScriptSoundNameView {
    char padding0[8];
    int soundStorageFlags;
    char paddingC[4];
    void* soundNameData;
};

struct BgmStopCommandView {
    char padding0[4];
    unsigned short startFrame;
    char padding6[2];
    float fadeSeconds;
};

struct Unk802232Data { char padding0[2]; unsigned short first; unsigned int second; };
struct Unk802232Out : CEventConte { unsigned short first; float second; Unk802232Out(Unk802232Data* data); };
struct Unk802236Data { char padding0[2]; unsigned short first; unsigned int second; short value8; };
struct Unk802236Out : CEventConte { unsigned short first; unsigned char byte6; char padding7; short value8; unsigned int valueC; Unk802236Out(Unk802236Data* data); };
struct Unk80222CtorData { char padding0[2]; unsigned short first; unsigned int second; char padding8[4]; unsigned int third; };
struct Unk80222CtorOut { void* vtable; unsigned short first; char padding6[2]; unsigned int field8; unsigned int fieldC; void* field10; float value14; };

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
    float samples[32][3]; int sampleCount; char padding184[0x0C]; int mirrorXAxis;
    float shortHistory[30][3]; int shortHistoryIndex; float field300; float field304[2]; int swingAngle;
    char padding310[0x5A0]; int longHistoryIndex; char padding8B4[0x3070];
    virtual void slot00(); virtual void slot01(); virtual void slot02(); virtual void slot03();
    virtual void slot04(); virtual void slot05(); virtual void slot06(); virtual void slot07();
    virtual void slot08(); virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14(); virtual void slot15();
    virtual void slot16(); virtual void slot17(); virtual void slot18();
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
extern void gtl_CInputExDeviceBase_GetDeviceResult(void*, int*);
extern void gtl_CScreenPointer_GetSample(void*, int, void*);
extern void Vector_CopyComponents(void*, void*);

// Read and clamp the selected pad's samples, mirror X if needed, update both histories, and invoke swing analysis.
// arg is forwarded to the processing hook; dt is forwarded unchanged. Return 0
// with sampleCount=0 when no pad is selected; otherwise return 1. The resampler's
// count query must stay within 32 and stable for this update (concrete count: 10/12).
int CPadSwingAccele_ReadAndUpdateSwingHistory(SwingSampleUpdateView* self, void* arg, float dt);

// Decode the BGM-stop start frame and convert fade frames to seconds.

// Decode a BGM pitch ramp, retaining the initial pitch, clamped delta, and duration.

// Convert script frame counts for the active video timing mode, preserving zero and one.
inline unsigned int ScaleScriptFrameCount(unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (n < 2) return n;
        n = Runtime_ConvertDoubleToUnsigned((double)(kEventDurationScaleNumerator_SoundScript * (float)n / kEventDurationScaleDenominator_SoundScript));
    }
    return n;
}

// Read the stored string length from either inline or heap string storage.
inline unsigned int GetScriptSoundNameLength(Unk80222CtorOut* self) {
    unsigned int result;
    if ((self->field8 >> 31) == 0) result = *(unsigned char*)&self->field8 & 0x7f;
    else result = self->fieldC;
    return result;
}

// Replace the owned sound name by copying borrowed text through the string allocator ABI.
inline void ReplaceScriptSoundName(Unk80222CtorOut* self, const char* text, char* tags) {
    unsigned int capacity = GetScriptSoundNameLength(self);
    tags[8] = tags[12];
    ((void (*)(void*, int, unsigned int, const char*, const char*, char*))String_ReplaceRange)(&self->field8, 0, capacity, text, text + strlen(text), tags + 8);
}

// Append the resource-string suffix to the stored sound name, copying its bytes.
inline void AppendScriptSoundName(Unk80222CtorOut* self, char* tags) {
    unsigned int capacity;
    const char* text = scriptSoundResourceStrings;
    capacity = GetScriptSoundNameLength(self);
    tags[0] = tags[4];
    ((void (*)(void*, int, unsigned int, const char*, const char*, char*))String_ReplaceRange)(&self->field8, capacity, 0, text, text + strlen(text), tags);
}

} }
struct CEventSoundSC_SND_BGM_PLAY : CEventConte {
    unsigned short first; char padding6[2];
    std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > text;
    float value14;
    CEventSoundSC_SND_BGM_PLAY(ns1::Unk80222CtorData* data);
};
// Decode BGM playback timing, append the resource extension, and convert fade duration to seconds.

namespace ns1 { extern "C" {

struct BgmTransitionManagerView {
    char padding0[0x38];
    float fadeSeconds;
};

struct BgmRampCommandView {
    char padding0[4]; unsigned short startFrame; unsigned char initialValue; char padding7;
    short valueDelta; char paddingA[2]; unsigned int durationFrames;
};

// Interpolate BGM volume from the saved initial value, publish it, and finish at the duration limit.
int CEventSoundSC_SND_BGM_VOL_UpdateRamp(BgmRampCommandView* self, unsigned int inputSampleCount);

// Interpolate BGM pitch percent, publish it to the sound manager, and finish at the duration limit.
int CEventSoundSC_SND_BGM_PITCH_ApplyRamp(BgmRampCommandView* self, unsigned int inputSampleCount);

struct BgmVolumeCommandView { void* vtable; unsigned short startFrame; unsigned char initialVolume; char padding7; short volumeDelta; char paddingA[2]; unsigned int durationFrames; };
struct Unk802234Data { char padding0[2]; unsigned short first; unsigned int second; unsigned char third; };
} }
struct CEventSoundSC_SND_BGM_VOL : CEventConte {
    char data[12];
    CEventSoundSC_SND_BGM_VOL(ns1::Unk802234Data* data);
};
// Decode a BGM-volume ramp using the current scripted volume as its initial value.

namespace ns1 { extern "C" {

// Apply the fade duration, request a BGM stop, and reset script pitch and volume baselines.
int CEventSoundSC_SND_BGM_STOP_RequestBgmStop(BgmStopCommandView* self, unsigned int inputSampleCount);

// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_PITCH_dtor(ScriptSoundNameView* ptr, int deletingFlag);

// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_BGM_PLAY_dtor(ScriptSoundNameView* ptr, int deletingFlag);

// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_STOP_dtor(void* ptr, int deletingFlag);

// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_VOL_dtor(void* ptr, int deletingFlag);

// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_PITCH_dtor(void* ptr, int deletingFlag);

// Optionally delete the swing object; its sample buffers are embedded in the object.
void* CPadSwingAccele_dtor(void* ptr, int deletingFlag);

// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_REQ_dtor(ScriptSoundNameView* ptr, int deletingFlag);

// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_PLAY_dtor(ScriptSoundNameView* ptr, int deletingFlag);

// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_STOP_dtor(ScriptSoundNameView* ptr, int deletingFlag);

// Initialize this BGM/swing unit's full-turn constant.
void EventBgmSwing_InitFullTurn();

// At the start frame, request the named BGM with its fade, reset sound baselines, and clear the name.
int CEventSoundSC_SND_BGM_PLAY_RequestBgmChange(Unk80222Init* self, unsigned int inputSampleCount);

// Reset sample counts, both history rings, the angle, and the cached last resampling vector.
int CPadSwingAccele_ResetSwingState(SwingInitializationView* self);

// Initialize the swing interface and capture buffers, clear endpoint vectors, and run the reset hook.
SwingInitializationView* CPadSwingAccele_Ctor(SwingInitializationView* self);

} // extern "C"

} // namespace ns1

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
namespace ns2 {
// Translation unit 0x80224150-0x80225628 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern const float kFloat_1_SwingCapture;
extern const float kFloat_3_SwingInterpolation;
extern const float kFloat_2_9_PadSwing;
extern const float kFloat_neg2_9_PadSwing;
extern const float kFloat_0_5_SwingAccelerometer;
extern int CPadSwingAccele_RepairClippedAccelerationRuns(int, Vec3f*, Vec3f*, int);

extern const float kFloat_0_0_SwingAcceleration;
extern void Vec3_SetAll(void* ptr, float value);

struct SwingAngleAndRangeView {
    char padding0[0x30C];
    float swingAngle;
    char padding310[0x3908 - 0x310];
    void* accelerationInRange;
};

// Evaluate the Y component of a cubic Bezier curve without changing floating-point operation order.
inline float EvaluateCurve(Vec3f* p0, Vec3f* p1, Vec3f* p2, Vec3f* p3, float t) {
    float u = 1.0f - t;
    return u * (u * (p0->y * u)) + t * (u * (3.0f * p1->y * u))
        + t * (t * (3.0f * p2->y * u)) + t * (t * (p3->y * t));
}

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
#define PROCESS_AXIS(member, active, start) \
    if ((current->member < 2.9f && current->member > -2.9f) || i == last) { \
        if (!active) { \
            output->member = current->member; \
        } else { \
            output->member = current->member; \
            int end = i - 1; \
            int length = end - start + 1; \
            if (end < last - 1 && source[end].member >= 2.9f && source[end + 2].member >= 2.9f) { \
            } else if (end < last - 1 && source[end].member <= -2.9f && source[end + 2].member <= -2.9f) { \
            } else if (length < 3) { \
                for (int j = start; j <= end; ++j) destination[j].member = source[j].member; \
                active = 0; \
            } else { \
                if (start > 1) { \
                    p0.x = (float)(start - 1); \
                    p0.y = source[start - 1].member; \
                    p1.x = (float)length * 0.5f + (float)(start - 1); \
                    p1.y = (float)length * 0.5f * (source[start - 1].member - source[start - 2].member) + source[start - 1].member; \
                } else if (start == 1) { \
                    p0.x = (float)start; \
                    p0.y = source[start].member; \
                    p1.x = (float)length * 0.5f + (float)start; \
                    p1.y = (float)length * 0.5f * (source[start].member - source[start - 1].member) + source[start].member; \
                } else { \
                    p0.x = 0.0f; \
                    p0.y = source[0].member; \
                    p1.x = (float)length * 0.5f; \
                    p1.y = (float)length * 0.5f * (source[1].member - source[0].member) + source[0].member; \
                } \
                if (end < last - 1) { \
                    p3.x = (float)(end + 1); \
                    p3.y = source[end + 1].member; \
                    p2.x = (float)(end + 1) - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (source[end + 1].member - source[end + 2].member) + source[end + 1].member; \
                } else if (end == last - 1) { \
                    p3.x = (float)end; \
                    p3.y = source[end].member; \
                    p2.x = (float)end - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (source[end].member - source[end + 1].member) + source[end].member; \
                } else { \
                    p3.x = (float)last; \
                    p3.y = finalPoint->member; \
                    p2.x = (float)last - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (finalPoint[-1].member - finalPoint->member) + finalPoint->member; \
                } \
                Vec3f* target = destination + start; \
                for (int j = 0; j < length; ++j, ++target) { \
                    float t = (1.0f / (float)(length + 1)) * (float)(j + 1); \
                    target->member = EvaluateCurve(&p0, &p1, &p2, &p3, t); \
                } \
                active = 0; \
            } \
        } \
    } else if (!active) { \
        active = 1; \
        start = i; \
    }

// Repair clipped acceleration runs on each axis with a cubic Bezier interpolation of surrounding samples.
// Borrow source/count valid vectors and destination/count writable vectors, with
// count >= 1. Keep them separate: interpolation re-reads original neighboring
// samples while replacing earlier destination samples. Returns 1; does not
// allocate or clamp the interpolated results back to the input clipping range.
int CPadSwingAccele_RepairClippedAccelerationRuns(int unused, Vec3f* destination, Vec3f* source, int count);
#undef PROCESS_AXIS

// Return the current processed swing angle.
float CPadSwingAccele_vfunc_8(SwingAngleAndRangeView* self);

// Return the stored acceleration-range flag through the original pointer-sized interface.
void* CPadSwingAccele_vfunc_9(SwingAngleAndRangeView* self);

// Clear capture/corrected buffers, arm capture, save its frame limit, and reset peak and endpoint data.
int CPadSwingAccele_ArmCapture(void* object, void* value);

} // extern "C"

} // namespace ns2

// ---- merged from event_fx_sound_and_pad_swing_accele.cpp ----
namespace ns3 {
// Translation unit 0x80225628-0x80226C9C (21 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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

struct SwingAnalysisView {
    char padding0000[0x190];
    int mirrorXAxis;
    char padding0194[0x38F8 - 0x194];
    int peakSampleIndex;
    float swingDirectionAngle;
    float peakWidthSamples;
    float peakMeanMagnitude;
};

struct SwingSampleCountView {
    char padding0000[0x38F0];
    unsigned int capturedSampleCount;
};

struct SwingRangeQueryView {
    char padding0000[0x300];
};

struct SwingAbsoluteSumView {
    char padding0000[0x20C0];
    float correctedSampleX;
    float correctedSampleY;
    float correctedSampleZ;
    char padding20CC[0x38F0 - 0x20CC];
    int capturedSampleCount;
};

struct SwingPartialSumView {
    char padding0000[0x38F0];
    int capturedSampleCount;
    char padding38F4[0x30];
    void** vtable3924;
};
// slot54 returns the timing-mode resampling count; it is not capturedSampleCount.
struct Vtable54 { char unk[0x54]; int (*slot54)(void*); };
typedef Vec3f Vec3;

struct SwingMotionStateView {
    int motionState;
    float motionPosition;
    char padding0008[0x4];
    float motionVelocity;
    float motionAcceleration;
    char padding0014[0x18];
    int motionSettled;
    float settlingElapsed;
    int animationStarted;
    float animationElapsed;
    float completionElapsed;
    float animationStartWaitElapsed;
};

struct SwingMotionInitializationView {
    int motionState;
    float motionPosition;
    float motionLimit;
    float motionVelocity;
    float motionAcceleration;
    float motionStep;
    int ownedChannel0;
    int ownedChannel1;
    int ownedChannel2;
    int ownedChannel3;
    int ownedChannel4;
    int motionSettled;
    float settlingElapsed;
    int animationStarted;
    float animationElapsed;
    float completionElapsed;
    char padding0040[4];
    int motionVtable;
};

// Motion interface +60 queries the animation-table index; object interface +c0
// binds that named motion with a borrowed channel at controller +8. The full
// backend types are unresolved; these views establish only the used slots.
struct Vtable60 { char unk[0x60]; void (*slot60)(); };
struct Object44 { char unk[0x44]; Vtable60* vtable; };
struct VtableC0 { char unk[0xC0]; void (*slotC0)(void*, void*, const char*); };
struct Object0 { VtableC0* vtable; };

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

// Return mean vector magnitude around the detected swing peak.
float CPadSwingAccele_vfunc_12(SwingAnalysisView* self);

// Return the detected peak width measured in samples.
float CPadSwingAccele_vfunc_13(SwingAnalysisView* self);

// Return the handedness-adjusted direction angle found by the capture analysis.
float CPadSwingAccele_vfunc_14(SwingAnalysisView* self);

// Find the swing peak with five-sample windows, then calculate direction, mean magnitude, and peak width.
int CPadSwingAccele_AnalyzeCapturedSamples(SwingAnalysisView* self, Vec3Sample* v, int count);

// Initialize the motion state, timing, interface, and five owned channel pointers.
void CPadSwingCtrlMotion_Ctor(SwingMotionInitializationView* self);

// Copy the stored start/end capture vectors to two required output buffers.
int CPadSwingAccele_CopyCaptureAccelerations(void* self, const char* first, const char* second);

// Test current acceleration against the configured X/Y range and store the result.
int CPadSwingAccele_UpdateAccelerationInRangeFlag(SwingRangeQueryView* self);

#pragma use_lmw_stmw on
// Delete all five owned motion-channel arrays and optionally release the controller.
void* CPadSwingCtrlMotion_dtor(void* object, int shouldDelete);
#pragma use_lmw_stmw reset

// Accumulate absolute corrected acceleration per axis and return at least one as the divisor count.
int CPadSwingAccele_SumAbsoluteCorrectedSamples(SwingAbsoluteSumView* self, float* outX, float* outY, float* outZ, int* outCount);

// Accumulate absolute corrected acceleration over the requested capture fraction.
int CPadSwingAccele_SumAbsCorrectedSamplePrefix(SwingPartialSumView* self, int* outCount, Vec3* values, float fraction);

struct P44B { char pad[68]; };
class P44 : public P44B { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual int v60();
};
class PC0 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual void v74(); virtual void v78(); virtual void v7C(); virtual void v80(); virtual void v84(); virtual void v88(); virtual void v8C(); virtual void v90(); virtual void v94(); virtual void v98(); virtual void v9C(); virtual void vA0(); virtual void vA4(); virtual void vA8(); virtual void vAC(); virtual void vB0(); virtual void vB4(); virtual void vB8(); virtual void vBC(); virtual void vC0(char*,const char*);
};
// Bind the motion entry selected by the virtual motion-index query to the supplied object.
float CPadSwingCtrlMotion_BindSelectedMotion(void* self, Object0* object);

// Return the number of captured swing samples.
unsigned int CPadSwingAccele_vfunc_15(SwingSampleCountView* self);

// Return the current motion-state value.
unsigned int CPadSwingCtrlMotion_vfunc_1(unsigned int* self);

float s_swingMotionFullTurn;
extern const float kFloat_6_2831855;
extern int EventTiming_IsDurationScalingEnabled();

// Initialize this motion-controller unit's full-turn constant.
void SwingMotion_InitFullTurn();

// Advance the motion phase, clearing the timing and position data needed by its next phase.
int CPadSwingCtrlMotion_AdvanceMotionPhase(SwingMotionStateView* self);

// Return the timing-mode-dependent resampler output sample count.
int CPadSwingAccele_GetResampleOutputSampleCount();

// Cancel forward motion by switching to the return phase; report whether a change occurred.
int CPadSwingCtrlMotion_CancelForwardMotion(int* self);

// Return the motion controller to idle and clear its timing, position, and completion data.
int CPadSwingCtrlMotion_ResetMotionToIdle(SwingMotionStateView* self);

struct UpdateVirtualBase { char pad[0x3924]; };
// Capture dispatch: +24 repairs clipped runs (vfunc_7); +60 analyzes the repaired
// capture (vfunc_22). Other slots preserve ABI order and are not inferred here.
class UpdateVirtual : public UpdateVirtualBase {
public:
    virtual void v08(); virtual void v0C(); virtual void v10();
    virtual void v14(); virtual void v18(); virtual void v1C();
    virtual void v20(); virtual void v24(void*, void*, int);
    virtual void v28(); virtual void v2C(); virtual void v30();
    virtual void v34(); virtual void v38(); virtual void v3C();
    virtual void v40(); virtual void v44(); virtual void v48();
    virtual void v4C(); virtual void v50(); virtual void v54();
    virtual void v58(); virtual void v5C(); virtual void v60(void*, int);
};

// Capture samples until the threshold/time limit completes the swing, then repair and analyze the capture.
// delta is seconds. enabled gates edge detection only; appending and time-based
// completion still run when false. Return SwingCaptureState (not a success flag).
// Requires count valid newest-first values and two writable 512-vector buffers.
// v24/v60 retain their ABI names: these are repair/analyze dispatches, in that
// order, on both closing paths; output is owned by self and remains available.
int CPadSwingAccele_UpdateCapture(UpdateObject* self, int enabled, float delta);

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
int CPadSwingAccele_ResampleNewestFirstBatch(void* self, Vec3* output, Vec3* values, int inputSampleCount);

struct PadAccele {
    char pad0[0x180];
    int historyCount;
    char pad1[0x300 - 0x184];
    Vec3 current;
    float angle;
    Vec3 history[120];
    int head;
    float negativeYHoldTime;
    float negativeXHoldTime;
    float steadyAccelerationTime;
};

// Remove held/stationary acceleration bias, clamp axes, and derive the current swing angle.
// delta is seconds; enabled gates history-bias removal only. Always returns 1.
// value is a local processed acceleration copy: clamps/correction do not write
// current back. history/head/hold timers and angle are the modified members.
// angle is radians from +Y toward +X in the processed acceleration plane; the
// negative-X/negative-Y branches use Z or the preserved square-root expression.
int CPadSwingAccele_CorrectAccelAndUpdateAngle(PadAccele* self, int enabled, float delta);

} // extern "C"

} // namespace ns3

extern "C" { void EventSoundEffects_InitFullTurn(); void EventBgmSwing_InitFullTurn(); void SwingMotion_InitFullTurn(); }
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
extern "C" {
// Delete every owned event effect, free the map nodes, and optionally release the manager.
void* CEventEffect_dtor(void* self, int dealloc) {
    char* p = (char*)self;
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
                        if (value) ((ValueObject*)value)->vtbl->release(value, 1);
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
                        while ((par = reinterpret_cast<void*>((unsigned int)((TreeNode*)node)->parent & ~1u)), node != *reinterpret_cast<void**>(par)) {
                            node = par;
                        }
                        node = par;
                    }
                }
            }
            // Phase 2: free nodes after all payloads are gone, then reset the
            // root and cached first iterator to the empty-tree representation.
            DtorNode* first = (*(DtorNode**)&AsCEventEffect(p)->treeRoot);
            if (first) { destroyLevelTwoDtorNode(p, first->a); destroyLevelTwoDtorNode(p, first->b); appFreeUni__FPv(first); }
            *(void**)p = 0; AsCEventEffect(p)->treeRoot = 0; AsCEventEffect(p)->firstNode = p + 4;
        }
        // Phase 3: container/base cleanup also covers a count-zero tree. These
        // repeated receiver guards are the original nested destructor guards;
        // they protect one cleanup operation, not six different owned resources.
        if (p && p && p && p && p && p) {
            DtorNode* first = (*(DtorNode**)&AsCEventEffect(p)->treeRoot);
            if (first) { destroyLevelTwoDtorNode(p, first->a); destroyLevelTwoDtorNode(p, first->b); appFreeUni__FPv(first); }
        }
        if (dealloc > 0) __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

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
            if (node->key >= key) { candidate = node; node = node->left; }
            else node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key) candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) { delete value; candidate->value = 0; }
        }
    }
    EntryObject* object = new EntryObject(a, s_cameraTransform, 0, d, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct { unsigned char k; unsigned int v; } pr;
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
            if (node->key >= key) { candidate = node; node = node->left; }
            else node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key) candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) { delete value; candidate->value = 0; }
        }
    }
    EntryObject* object = new EntryObject(a, b + 0x50, 0, d, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct { unsigned char k; unsigned int v; } pr;
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
void EventFxMap_ReplaceFromNamedBone(Tree* self, unsigned char key, const char* a, void* data, const char* source, int extra) {
    void* descriptor = WPuppet_FindNamedBoneEntry(data, source);
    if (!descriptor) return;
    if (self->count) {
        TreeNode* node = *(TreeNode* volatile*)&self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        unsigned int k2;
        unsigned int kr = *(volatile unsigned char*)&key;
        k2 = *(volatile unsigned char*)&key;
        while (node) {
            if (node->key >= k2) { candidate = node; node = node->left; }
            else node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || kr < candidate->key) candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) {
                delete value;
                candidate->value = 0;
            }
        }
    }
    EntryObject* object = new EntryObject(a, ((BoneEffectPositionSource*)*(void**)descriptor)->positionData, (int)((char*)*((void**)descriptor + 1) + 0x18), extra, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct { unsigned char k; unsigned int v; } pr;
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
void EventFxMap_SetBoneAttachedEffect(Tree* self, unsigned char key, const char* a, void* data, const char* source, int extra) {
    void* descriptor = WPuppet_FindOrCacheBone(data, source);
    if (!descriptor) return;
    if (self->count) {
        TreeNode* node = *(TreeNode* volatile*)&self->treeRoot;
        TreeNode* candidate = (TreeNode*)&self->treeRoot;
        unsigned int k2;
        unsigned int kr = *(volatile unsigned char*)&key;
        k2 = *(volatile unsigned char*)&key;
        while (node) {
            if (node->key >= k2) { candidate = node; node = node->left; }
            else node = node->right;
        }
        if (candidate == (TreeNode*)&self->treeRoot || kr < candidate->key) candidate = (TreeNode*)&self->treeRoot;
        if (candidate != (TreeNode*)&self->treeRoot) {
            VirtValue* value = (VirtValue*)candidate->value;
            if (value) { delete value; candidate->value = 0; }
        }
    }
    EntryObject* object = new EntryObject(a, ((BoneEffectPositionSource*)descriptor)->positionData, 0, extra, 0);
    unsigned char f, e;
    char* hint;
    void* found = EventFxMap_FindInsertionPosition(self, &key, (unsigned int*)&hint, &e, &f);
    char* slot = hint;
    if (!slot || lessEntryKey(*(unsigned char*)(slot += 0xC), key)) {
        struct { unsigned char k; unsigned int v; } pr;
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
    if (self->count == 0) return;
    TreeNode* node = self->treeRoot;
    candidate = (TreeNode*)&self->treeRoot;
    while (node) {
        if (node->key >= key) { candidate = node; node = node->left; }
        else node = node->right;
    }
    if (candidate == (TreeNode*)&self->treeRoot || key < candidate->key) candidate = (TreeNode*)&self->treeRoot;
    if (candidate == (TreeNode*)&self->treeRoot) return;
    VirtValue* value = (VirtValue*)candidate->value;
    if (value) { delete value; candidate->value = 0; }
    erased = candidate;
    if (erased == self->firstNode) {
        TreeNode* x = candidate->right;
        if (x) {
            while (x->left) x = x->left;
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
        if ((*(int*)&((CEventFx*)r30)->spray) != 0 &&
            (*(int*)&((CEventFx*)r30)->sequence) != 0 &&
            (*(int*)&((CEventFx*)r30)->positionOffset) != 0) {
            WVector tmp = *(WVector*)(((CEventFxPositionLayout*)r30)->basePosition) + *(WVector*)(reinterpret_cast<const float*>(((CEventFx*)r30)->positionOffset));
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
            while ((par = reinterpret_cast<void*>((unsigned int)((TreeNode*)node)->parent & ~1u)), node != *reinterpret_cast<void**>(par)) {
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
extern "C" {
// Optionally delete a fade command that owns no additional resources.
void* CEventEffectSC_EFF_FADE_dtor(void* self, int dealloc) {
    if (self != 0 && dealloc > 0) {
        __dl__FPv(self);
    }
    return self;
}
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
extern "C" {
// At the start frame, request the named sound once, reset pitch, and clear the command's name.
int CEventSoundSC_SND_SE_REQ_PlayNamedSoundOnce(ScriptSoundCommandView* self, unsigned int frame) {
    if (frame < self->startFrame) {
        return 0;
    }
    void* soundName;
    unsigned int bit = (unsigned int)self->soundStorageFlags >> 31;
    if (bit == 0) {
        soundName = (char*)&AsCEventSoundSC(self)->soundNameFirstChar;
    } else {
        soundName = self->soundNameData;
    }
    Sound_PlayByName(g_soundManager, (const char*)soundName, 0, 0, (const char*)0, 0, kFloat_0_5_EventSound, kFloat_200_EventFx, kFloat_1_EventFx);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    bit = (unsigned int)self->soundStorageFlags >> 31;
    ClearSoundCommandName(self, bit);
    return 1;
}
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
extern "C" {
// At the start frame, play the sound with its alias/loop flag, then reset pitch and clear the name.
int CEventSoundSC_SND_SE_PLAY_TryPlaySoundEffect(ScriptSoundCommandView* self, unsigned int frame) {
    if (frame < self->startFrame) {
        return 0;
    }
    unsigned int inv = !((unsigned int)self->soundStorageFlags >> 31);
    const char* alias;
    if (inv) {
        alias = (const char*)&AsCEventSoundSC(self)->soundNameFirstChar;
    } else {
        alias = (const char*)self->soundNameData;
    }
    void* manager = g_soundManager;
    const char* soundName;
    if (inv != 0) {
        soundName = (const char*)&AsCEventSoundSC(self)->soundNameFirstChar;
    } else {
        soundName = (const char*)self->soundNameData;
    }
    register unsigned int loopFlag = self->loopFlag;
    ((void (*)(void*, const char*, int, int, const char*, int, float, float, float))Sound_PlayByName)(manager, soundName, 0, loopFlag, alias, 0, kFloat_0_5_EventSound, kFloat_200_EventFx, kFloat_1_EventFx);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    ClearSoundCommandName(self, (unsigned int)self->soundStorageFlags >> 31);
    return 1;
}
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
extern "C" {
// At the start frame, stop the named sound alias, reset pitch, and clear the command's name.
int CEventSoundSC_SND_SE_STOP_StopSoundAlias(ScriptSoundCommandView* self, unsigned int frame) {
    if (frame < self->startFrame) {
        return 0;
    }
    void* soundName;
    unsigned int bit = (unsigned int)self->soundStorageFlags >> 31;
    if (bit == 0) {
        soundName = (char*)&AsCEventSoundSC(self)->soundNameFirstChar;
    } else {
        soundName = self->soundNameData;
    }
    Sound_StopByAlias(g_soundManager, (const char*)soundName);
    s_eventSoundEffectPitchPercent = kPitchUnityPercent;
    bit = (unsigned int)self->soundStorageFlags >> 31;
    ClearSoundCommandName(self, bit);
    return 1;
}
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
    if ((text.flags >> 31) == 0) capacity = *(unsigned char*)&text & 0x7f;
    else capacity = text.length;
    volatile struct { char value; char padding[3]; } unused;
    char local = unused.value;
    unsigned int length = strlen(str);
    String_ReplaceRange(&text, 0, capacity, str, str + length, &local);
    *(volatile unsigned char*)&this->startPitchPercent = s_eventSoundEffectPitchPercent;
    short amount = *(const volatile short*)(arg + 8);
    this->pitchDeltaPercent = amount;
    if (amount > kMaximumPitchDelta) this->pitchDeltaPercent = kMaximumPitchDelta;
    if (this->pitchDeltaPercent < kMinimumPitchDelta) this->pitchDeltaPercent = kMinimumPitchDelta;
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
extern "C" {
// Interpolate named-effect pitch from its initial percent over the scripted duration, then clear its name.
int CEventSoundSC_SND_SE_PITCH_UpdateEffectPitch(void* self, unsigned int value) { unsigned short base = AsCEventSoundSC(self)->startFrame; float interp;

    if (value < base) return 0;

    unsigned int delta = value - base;
    if (AsCEventSoundSC(self)->pitchDurationFrames != 0) {
        interp = (float)delta / (float)AsCEventSoundSC(self)->pitchDurationFrames;
    } else {
        interp = kFloat_1_EventFx;
    }
    short start = AsCEventSoundSC(self)->pitchDeltaPercent;
    unsigned char initialPitchPercent = AsCEventSoundSC(self)->initialPitchPercent; // pitch-command variant at +0x14
    unsigned int result = (int)((float)start * interp + (float)initialPitchPercent);
    s_eventSoundEffectPitchPercent = (unsigned char)result;
    float ratio = (float)(unsigned char)result / 100.0f;
    SoundManager_SetNamedEffectPitch(g_soundManager,
        ((AsCEventSoundSC_SND_SE_PITCH(self)->soundNameStorageFlags >> 31) == 0) ? (char*)&AsCEventSoundSC(self)->soundNameFirstChar : (*(char**)&AsCEventSoundSC(self)->soundNameData),
        ratio);
    if (delta < AsCEventSoundSC(self)->pitchDurationFrames) return 0;
    if (!(AsCEventSoundSC_SND_SE_PITCH(self)->soundNameStorageFlags >> 31)) {
        signed char* p = (signed char*)self;
        (*(signed char*)&AsCEventSoundSC(p)->soundNameFirstChar) = 0;
        AsCEventSoundSC(p)->soundNameStorageFlags &= (signed char)0x80;
    } else {
        *(*(unsigned char**)&AsCEventSoundSC(self)->soundNameData) = 0;
        AsCEventSoundSC(self)->soundNameLength = 0;
    }
    return 1;
}
}
#pragma pop

}


#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Scale the BGM playback start frame and copy its resource name into owned storage.
CEventSoundSC_SND_BGM_PLAY::CEventSoundSC_SND_BGM_PLAY(ns1::Unk80222CtorData* data) {
    using namespace ns1;
    Unk80222CtorOut* self = (Unk80222CtorOut*)this;
    self->field8 = 0; self->vtable = (void*)__vt__26CEventSoundSC_SND_BGM_PLAY; self->fieldC = 0; self->field10 = 0;
    unsigned int first = ScaleScriptFrameCount(data->first);
    self->first = first;
    const char* text = (const char*)data + 8;
    char tags[16];
    ReplaceScriptSoundName(self, text, tags);
    AppendScriptSoundName(self, tags);
    unsigned int second = ScaleScriptFrameCount(data->second);
    self->value14 = (float)second;
    if (EventTiming_IsDurationScalingEnabled()) self->value14 /= kFloat_25_EventSound;
    else self->value14 /= kFloat_30_EventSound;
}
#pragma pop


namespace ns1 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// At the start frame, request the named BGM with its fade, reset sound baselines, and clear the name.
int CEventSoundSC_SND_BGM_PLAY_RequestBgmChange(Unk80222Init* self, unsigned int index) {
    if (index < self->startFrame) return 0;
    ((SoundTransitionManagerView*)g_soundManager)->fadeSeconds = self->fadeSeconds;
    const char* text;
    if (((unsigned int)self->soundStorageFlags >> 31) == 0) text = (const char*)&AsCEventSoundSC(self)->soundNameFirstChar;
    else text = (const char*)self->soundNameData;
    SoundManager_RequestBgmChange(g_soundManager, text, 0, 1);
    s_scriptBgmPitchPercent = kPitchUnityPercent;
    float value = SoundManager_GetVolumeByMode(g_soundManager, 0);
    s_eventBgmVolumeStorage = (unsigned char)(kFloat_255_StreamVolume * value);
    if (((unsigned int)self->soundStorageFlags >> 31) == 0) {
        AsCEventSoundSC(self)->soundNameFirstChar = 0;
        AsCEventSoundSC(self)->soundNameStorageFlags &= (signed char)0x80;
    } else {
        *(unsigned char*)self->soundNameData = 0;
        self->soundNameLength = 0;
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the scaled BGM stop start frame and its fade duration.
Unk802232Out::Unk802232Out(Unk802232Data* data) {
    Unk802232Out* self = this;
    self->vtable = (void*)__vt__26CEventSoundSC_SND_BGM_STOP;
    unsigned int first = data->first;
    if (EventTiming_IsDurationScalingEnabled()) {
        first = (first < 2) ? first : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)first / kEventDurationScaleDenominator_SoundScript);
    }
    self->first = first;
    unsigned int second = data->second;
    if (EventTiming_IsDurationScalingEnabled()) {
        second = (second < 2) ? second : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)second / kEventDurationScaleDenominator_SoundScript);
    }
    self->second = (float)second;
    if (EventTiming_IsDurationScalingEnabled()) self->second /= kFloat_25_EventSound;
    else self->second /= kFloat_30_EventSound;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Apply the fade duration, request a BGM stop, and reset script pitch and volume baselines.
int CEventSoundSC_SND_BGM_STOP_RequestBgmStop(BgmStopCommandView* self, unsigned int index) {
    if (index < self->startFrame) {
        return 0;
    }
    ((BgmTransitionManagerView*)g_soundManager)->fadeSeconds = self->fadeSeconds;
    SoundManager_RequestBgmStop(g_soundManager, 1);
    s_scriptBgmPitchPercent = kPitchUnityPercent;
    float value = SoundManager_GetVolumeByMode(g_soundManager, 0);
    s_eventBgmVolumeStorage = (unsigned char)(kFloat_255_StreamVolume * value);
    return 1;
}
}
#pragma pop

}


#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the BGM volume ramp's initial byte value, target delta and scaled timing.
CEventSoundSC_SND_BGM_VOL::CEventSoundSC_SND_BGM_VOL(ns1::Unk802234Data* data) {
    using namespace ns1;
    BgmVolumeCommandView* self = (BgmVolumeCommandView*)this; self->vtable = (void*)__vt__25CEventSoundSC_SND_BGM_VOL;
    unsigned int first = ScaleScriptFrameCount(data->first);
    self->startFrame = (unsigned short)first;
    unsigned char clamped = 0xff;
    unsigned char value = s_eventBgmVolumeStorage;
    self->initialVolume = value;
    unsigned char end = data->third;
    if (end < 0xff) clamped = end;
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
extern "C" {
// Interpolate BGM volume from the saved initial value, publish it, and finish at the duration limit.
int CEventSoundSC_SND_BGM_VOL_UpdateRamp(BgmRampCommandView* self, unsigned int index) {
    if (index < self->startFrame) return 0;
    unsigned int n = index - self->startFrame;
    float ratio;
    if (self->durationFrames != 0) ratio = (float)n / (float)self->durationFrames;
    else ratio = kFloat_1_EventFx;
    int value = (int)((float)self->valueDelta * ratio + (float)self->initialValue);
    s_eventBgmVolumeStorage = value;
    unsigned char byte = (unsigned char)value;
    SoundManager_ScaleStreamVolume(g_soundManager, (float)byte / kFloat_255_StreamVolume);
    if (n < self->durationFrames) return 0;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
// Capture the BGM pitch ramp relative to its current percent value, clamping the target.
Unk802236Out::Unk802236Out(Unk802236Data* data) {
    Unk802236Out* self = this;
    self->vtable = (void*)__vt__27CEventSoundSC_SND_BGM_PITCH;
    unsigned int first = data->first;
    if (EventTiming_IsDurationScalingEnabled()) {
        first = first < 2 ? first : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)first / kEventDurationScaleDenominator_SoundScript);
    }
    self->first = first;
    self->byte6 = s_scriptBgmPitchPercent;
    short value = data->value8;
    self->value8 = value;
    if (self->value8 > kMaximumPitchDelta) self->value8 = kMaximumPitchDelta;
    if (self->value8 < -100) self->value8 = -100;
    self->value8 += 100;
    self->value8 -= self->byte6;
    unsigned int second = data->second;
    if (EventTiming_IsDurationScalingEnabled()) {
        second = second < 2 ? second : Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_SoundScript * (float)second / kEventDurationScaleDenominator_SoundScript);
    }
    self->valueC = second;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Interpolate BGM pitch percent, publish it to the sound manager, and finish at the duration limit.
int CEventSoundSC_SND_BGM_PITCH_ApplyRamp(BgmRampCommandView* self, unsigned int index) {
    if (index < self->startFrame) return 0;
    unsigned int n = index - self->startFrame;
    float ratio;
    if (self->durationFrames != 0) ratio = (float)n / (float)self->durationFrames;
    else ratio = kFloat_1_EventFx;
    int value = (int)((float)self->valueDelta * ratio + (float)self->initialValue);
    s_scriptBgmPitchPercent = value;
    unsigned char byte = (unsigned char)value;
    SoundManager_SetStreamPitch(g_soundManager, (float)byte / kPitchPercentScale_BgmScript);
    if (n < self->durationFrames) return 0;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_REQ_dtor(ScriptSoundNameView* ptr, int deletingFlag) {
    if (ptr != 0) {
        if (&ptr->soundStorageFlags != 0 && ((unsigned int)ptr->soundStorageFlags >> 31) != 0) {
            appFreeUni__FPv(ptr->soundNameData);
        }
        if (deletingFlag > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_PLAY_dtor(ScriptSoundNameView* ptr, int deletingFlag) {
    if (ptr != 0) {
        if (&ptr->soundStorageFlags != 0 && ((unsigned int)ptr->soundStorageFlags >> 31) != 0) {
            appFreeUni__FPv(ptr->soundNameData);
        }
        if (deletingFlag > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_STOP_dtor(ScriptSoundNameView* ptr, int deletingFlag) {
    if (ptr != 0) {
        if (&ptr->soundStorageFlags != 0 && ((unsigned int)ptr->soundStorageFlags >> 31) != 0) {
            appFreeUni__FPv(ptr->soundNameData);
        }
        if (deletingFlag > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_SE_PITCH_dtor(ScriptSoundNameView* ptr, int deletingFlag) {
    if (ptr != 0) {
        if (&ptr->soundStorageFlags != 0 && ((unsigned int)ptr->soundStorageFlags >> 31) != 0) {
            appFreeUni__FPv(ptr->soundNameData);
        }
        if (deletingFlag > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Release any heap-backed sound name and optionally delete this script command.
void* CEventSoundSC_SND_BGM_PLAY_dtor(ScriptSoundNameView* ptr, int deletingFlag) {
    if (ptr != 0) {
        if (&ptr->soundStorageFlags != 0 && ((unsigned int)ptr->soundStorageFlags >> 31) != 0) {
            appFreeUni__FPv(ptr->soundNameData);
        }
        if (deletingFlag > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_STOP_dtor(void* ptr, int deletingFlag) {
    if (ptr != 0 && deletingFlag > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_VOL_dtor(void* ptr, int deletingFlag) {
    if (ptr != 0 && deletingFlag > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Optionally delete this script-command wrapper; it owns no sound-name storage.
void* CEventSoundSC_SND_BGM_PITCH_dtor(void* ptr, int deletingFlag) {
    if (ptr != 0 && deletingFlag > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Initialize this BGM/swing unit's full-turn constant.
void EventBgmSwing_InitFullTurn() { s_eventBgmSwingFullTurn = kFloat_6_2831855; }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Initialize the swing interface and capture buffers, clear endpoint vectors, and run the reset hook.
SwingInitializationView* CPadSwingAccele_Ctor(SwingInitializationView* self) {
    self->mirrorXAxis = 0;
    self->vtable = (SwingInterfaceVtable*)__vt__15CPadSwingAccele;
    self->capturedSampleCount = 0;
    Vec3_SetAll((char*)AsCPadSwingAccele(self)->captureStartAcceleration, kFloat_0_0_SwingAcceleration);
    Vec3_SetAll((char*)AsCPadSwingAccele(self)->captureEndAcceleration, kFloat_0_0_SwingAcceleration);
    char* p1 = (char*)AsCPadSwingAccele(self)->capturedSamples;
    char* p2 = ((char*)&AsCPadSwingAccele(self)->correctedSampleX);
    for (int i = 0; i < kCaptureCapacity; ++i) {
        Vec3_SetAll(p1, kFloat_0_0_SwingAcceleration);
        Vec3_SetAll(p2, kFloat_0_0_SwingAcceleration);
        p1 += 0xC;
        p2 += 0xC;
    }
    Vec3_SetAll((char*)AsCPadSwingAccele(self)->currentAcceleration, kFloat_0_0_SwingAcceleration);
    ((SwingVirtual*)self)->init();
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Optionally delete the swing object; its sample buffers are embedded in the object.
void* CPadSwingAccele_dtor(void* ptr, int deletingFlag) {
    if (ptr != 0 && deletingFlag > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Reset sample counts, both history rings, the angle, and the cached last resampling vector.
int CPadSwingAccele_ResetSwingState(SwingInitializationView* self) {
    char* p;
    int i;
    for (i = 0, p = (char*)AsCPadSwingAccele(self)->shortHistory; i < kShortHistorySamples; ++i, p += 0xc) {
        Vec3_SetAll(p, kFloat_0_0_SwingAcceleration);
    }
    float f0 = kFloat_0_0_SwingAcceleration;
    AsCPadSwingAccele(self)->shortHistoryIndex = 0;
    int longIndex;
    char* longCursor;
    longCursor = (char*)AsCPadSwingAccele(self)->longHistory;
    longIndex = 0;
    self->swingAngle = f0;
    self->sampleCount = 0;
    while (longIndex < kLongHistorySamples) {
        Vec3_SetAll(longCursor, kFloat_0_0_SwingAcceleration);
        ++longIndex;
        longCursor += 0xc;
    }
    float f = kFloat_0_0_SwingAcceleration;
    char* q = (char*)self + SWING_OFFSET(lastResampledVector) + (AsCPadSwingAccele(self)->longHistoryIndex = 0);
    AsCPadSwingAccele(self)->negativeYHoldTime = f;
    AsCPadSwingAccele(self)->negativeXHoldTime = f;
    AsCPadSwingAccele(self)->steadyAccelerationTime = f;
    self->captureState = kCaptureIdle;
    self->captureFrameLimit = 0;
    self->accelerationInRange = 0;
    Vec3_SetAll(q, f);
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Read and clamp the selected pad's samples, mirror X if needed, update both histories, and invoke swing analysis.
int CPadSwingAccele_ReadAndUpdateSwingHistory(SwingSampleUpdateView* self, void* arg, float dt) {
    float* sp;
    int sampleIndex;
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_swingInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    register void* manager = InputManager_GetSelectedPad((void*)((int&)Sing_805DC5C8()));
    if (!manager) { self->sampleCount = 0; return 0; }
    gtl_CInputExDeviceBase_GetDeviceResult(manager, &self->sampleCount);
    // Acquire at most one raw batch; the update hook copies/resamples stack data.
    float samples[kSampleBatchCapacity][3];
    sampleIndex = 0;
    sp = (float*)samples;
    for (; sampleIndex < self->sampleCount; ) {
        if (sampleIndex >= kSampleBatchCapacity) { self->sampleCount = kSampleBatchCapacity; break; }
        float tmp[10];
        gtl_CScreenPointer_GetSample(manager, sampleIndex, tmp);
        Vector_CopyComponents(sp, tmp);
        sp += 3;
        ++sampleIndex;
    }
    // Resample first, then clamp every output axis and apply handedness to X.
    self->update(self, samples, self->sampleCount);
    self->sampleCount = self->getCount();
    float* cp = (float*)self;
    float lower;
    float upper;
    int clampIndex;
    upper = kFloat_2_9_PadSwing;
    clampIndex = 0;
    lower = kFloat_neg2_9_PadSwing;
    for (; clampIndex < self->sampleCount; ++clampIndex) {
        if (cp[0] > upper) cp[0] = upper;
        else if (cp[0] < lower) cp[0] = lower;
        if (cp[1] > upper) cp[1] = upper;
        else if (cp[1] < lower) cp[1] = lower;
        if (cp[2] > upper) cp[2] = upper;
        else if (cp[2] < lower) cp[2] = lower;
        cp += 3;
    }
    if (self->mirrorXAxis) {
        const float scale = kFloat_neg1_swingAccelerometer;
        for (int i = 0; i < self->sampleCount; ++i) {
            self->samples[i][0] *= scale;
        }
    }
    // Append oldest-to-newest into both rings; indices wrap at their own capacities.
    for (int i = 0; i < self->sampleCount; ++i) {
        Vector_CopyComponents(self->shortHistory[self->shortHistoryIndex], self->samples[self->sampleCount - i - 1]);
        ++self->shortHistoryIndex;
        if (self->shortHistoryIndex >= kShortHistorySamples) self->shortHistoryIndex %= kShortHistorySamples;
        Vector_CopyComponents((char*)self + self->longHistoryIndex * kSwingVectorBytes + kLongHistoryOffset, (char*)self + (self->sampleCount - i - 1) * kSwingVectorBytes);
        ++self->longHistoryIndex;
        if (self->longHistoryIndex >= kLongHistorySamples) self->longHistoryIndex %= kLongHistorySamples;
    }
    // The mean includes zero-initialized history slots during startup. Store X,Z,Y,
    // then run bias/angle processing and the ready-range query in that order.
    float x = kFloat_0_0_SwingAcceleration, y = x, z = x;
    for (int i = 0; i < kShortHistorySamples; ++i) { x += self->shortHistory[i][0]; y += self->shortHistory[i][1]; z += self->shortHistory[i][2]; }
    const float divisor = kFloat_30_PadSwing;
    self->field300 = x / divisor;
    self->field304[0] = z / divisor;
    self->field304[1] = y / divisor;
    self->draw(arg, dt);
    self->post();
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Report that the swing acceleration implementation is available.
int CPadSwingAccele_vfunc_3() { return 1; }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Set whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele_SetMirrorXAxis(SwingHandednessView* self, int value) {
    self->mirrorXAxis = value;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return whether sampled X acceleration is mirrored for the current handedness.
int CPadSwingAccele_vfunc_5(SwingHandednessView* self) { return self->mirrorXAxis; }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Toggle X mirroring and return the resulting handedness flag.
int CPadSwingAccele_ToggleMirrorXAxis(SwingHandednessView* self) {
    if (self->mirrorXAxis != 0) {
        self->mirrorXAxis = 0;
    } else {
        self->mirrorXAxis = 1;
    }
    return self->mirrorXAxis;
}
}
#pragma pop

}

namespace ns2 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
#define PROCESS_AXIS(member, active, start) \
    if ((current->member < 2.9f && current->member > -2.9f) || i == last) { \
        if (!active) { \
            output->member = current->member; \
        } else { \
            output->member = current->member; \
            int end = i - 1; \
            int length = end - start + 1; \
            if (end < last - 1 && source[end].member >= 2.9f && source[end + 2].member >= 2.9f) { \
            } else if (end < last - 1 && source[end].member <= -2.9f && source[end + 2].member <= -2.9f) { \
            } else if (length < 3) { \
                for (int j = start; j <= end; ++j) destination[j].member = source[j].member; \
                active = 0; \
            } else { \
                if (start > 1) { \
                    p0.x = (float)(start - 1); \
                    p0.y = source[start - 1].member; \
                    p1.x = (float)length * 0.5f + (float)(start - 1); \
                    p1.y = (float)length * 0.5f * (source[start - 1].member - source[start - 2].member) + source[start - 1].member; \
                } else if (start == 1) { \
                    p0.x = (float)start; \
                    p0.y = source[start].member; \
                    p1.x = (float)length * 0.5f + (float)start; \
                    p1.y = (float)length * 0.5f * (source[start].member - source[start - 1].member) + source[start].member; \
                } else { \
                    p0.x = 0.0f; \
                    p0.y = source[0].member; \
                    p1.x = (float)length * 0.5f; \
                    p1.y = (float)length * 0.5f * (source[1].member - source[0].member) + source[0].member; \
                } \
                if (end < last - 1) { \
                    p3.x = (float)(end + 1); \
                    p3.y = source[end + 1].member; \
                    p2.x = (float)(end + 1) - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (source[end + 1].member - source[end + 2].member) + source[end + 1].member; \
                } else if (end == last - 1) { \
                    p3.x = (float)end; \
                    p3.y = source[end].member; \
                    p2.x = (float)end - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (source[end].member - source[end + 1].member) + source[end].member; \
                } else { \
                    p3.x = (float)last; \
                    p3.y = finalPoint->member; \
                    p2.x = (float)last - (float)length * 0.5f; \
                    p2.y = (float)length * 0.5f * (finalPoint[-1].member - finalPoint->member) + finalPoint->member; \
                } \
                Vec3f* target = destination + start; \
                for (int j = 0; j < length; ++j, ++target) { \
                    float t = (1.0f / (float)(length + 1)) * (float)(j + 1); \
                    target->member = EvaluateCurve(&p0, &p1, &p2, &p3, t); \
                } \
                active = 0; \
            } \
        } \
    } else if (!active) { \
        active = 1; \
        start = i; \
    }
extern "C" {
// Repair clipped acceleration runs on each axis with a cubic Bezier interpolation of surrounding samples.
int CPadSwingAccele_RepairClippedAccelerationRuns(int unused, Vec3f* destination, Vec3f* source, int count) {
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
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return the current processed swing angle.
float CPadSwingAccele_vfunc_8(SwingAngleAndRangeView* self) { return self->swingAngle; }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return the stored acceleration-range flag through the original pointer-sized interface.
void* CPadSwingAccele_vfunc_9(SwingAngleAndRangeView* self) { return self->accelerationInRange; }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Clear capture/corrected buffers, arm capture, save its frame limit, and reset peak and endpoint data.
int CPadSwingAccele_ArmCapture(void* object, void* value) {
    char* base = (char*)object;
    char* first = (char*)AsCPadSwingAccele(base)->capturedSamples;
    char* second = (char*)&AsCPadSwingAccele(base)->correctedSampleX;
    for (int i = 0; i < kCaptureCapacity; ++i) {
        Vec3_SetAll(first, kFloat_0_0_SwingAcceleration);
        Vec3_SetAll(second, kFloat_0_0_SwingAcceleration);
        first += 0xC;
        second += 0xC;
    }
    register float zero = kFloat_0_0_SwingAcceleration;
    char* arg0;
    AsCPadSwingAccele(base)->captureFrameLimit = value;
    arg0 = (char*)AsCPadSwingAccele(base)->captureStartAcceleration;
    AsCPadSwingAccele(base)->peakAccelerationX0 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX1 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX2 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX3 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX4 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX5 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX6 = zero;
    AsCPadSwingAccele(base)->peakAccelerationX7 = zero;
    AsCPadSwingAccele(base)->captureState = kCaptureArmed;
    AsCPadSwingAccele(base)->capturedSampleCount = 0;
    AsCPadSwingAccele(base)->peakSampleIndex = 0;
    AsCPadSwingAccele(base)->peakHistoryIndex = 0;
    AsCPadSwingAccele(base)->captureZThresholdReached = 0;
    Vec3_SetAll((void*)arg0, zero);
    Vec3_SetAll((char*)AsCPadSwingAccele(base)->captureEndAcceleration, kFloat_0_0_SwingAcceleration);
    return 1;
}
}
#pragma pop

}

namespace ns3 {

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Capture samples until the threshold/time limit completes the swing, then repair and analyze the capture.
int CPadSwingAccele_UpdateCapture(UpdateObject* self, int enabled, float delta) {
    // Idle/completed captures do nothing; save the start acceleration once.
    int state = self->captureState;
    if (state == kCaptureComplete || state == kCaptureIdle) return state;
    if (self->capturedSampleCount == 0)
        Vector_CopyComponents((const char*)&self->captureStartAcceleration, (char*)&self->currentAcceleration);

    // Scan the newest-first batch backward to append chronological raw samples.
    // Keep one buffer slot available for the threshold-crossing closing sample.
    float zero;
    float threshold = kCaptureZThreshold_Swing;
    zero = kFloat_0_0_SwingAcceleration;
    Vec3* current = &self->values[self->count - 1];
    Vec3* prior = current;
    int index = self->count - 1;
    for (; index >= 0; --current, --prior, --index) {
        if (current->z > threshold) {
            int n = self->capturedSampleCount;
            if (n < kCaptureReservedEndpoint) {
                Vector_CopyComponents((const char*)&self->samples[n], (char*)prior);
                self->capturedSampleCount++;
                continue;
            }
        }
        if (self->captureState == kCaptureTriggered) {
            Vector_CopyComponents((const char*)&self->samples[self->capturedSampleCount], (char*)&self->values[index]);
            self->capturedSampleCount = self->capturedSampleCount + 1;
            ((UpdateVirtual*)self)->v24(self->output, self->samples, self->capturedSampleCount);
            ((UpdateVirtual*)self)->v60(self->output, self->capturedSampleCount);
            self->captureState = kCaptureComplete;
            Vector_CopyComponents((const char*)&self->captureEndAcceleration, (char*)&self->currentAcceleration);
            break;
        } else {
            self->capturedSampleCount = 0;
            self->captureElapsed = zero;
        }
    }

    // Enabled edge detection: maintain eight batch maxima and compare the
    // current minimum after Z crosses its threshold. An already triggered
    // capture takes the original cancellation path and clears its sample count.
    if (enabled) {
        if (self->captureState != kCaptureTriggered) {
            float max = kFloat_0_0_SwingAcceleration;
            Vec3* p = self->values;
            for (int i = self->count; i > 0; --i, ++p) {
                if (p->x > max) max = p->x;
            }
            self->maxes[self->peakHistoryIndex] = max;
            self->peakHistoryIndex++;
            if (self->peakHistoryIndex >= kPeakHistorySize) self->peakHistoryIndex = 0;
            float min = kFloat_1_SwingCapture;
            Vec3* q = self->values;
            for (int i = 0; i < self->count; ++q, ++i) {
                if (q->x < min) min = q->x;
                if (q->z > kCaptureTriggerZ) self->captureZThresholdReached = 1;
            }
            if (self->captureZThresholdReached != 0) {
                for (int i = 0; i < kPeakHistorySize; ++i) {
                    if (self->maxes[i] - min > kCapturePeakToTroughX) self->captureState = kCaptureTriggered;
                }
            }
        } else {
            self->captureState = kCaptureComplete;
            self->capturedSampleCount = 0;
        }
    }
    // Time fallback uses elapsed seconds * 30 to test frame limits; zero limit
    // allows an open-ended capture. A timed end uses a zero endpoint vector.
    if (self->captureState != kCaptureComplete && self->capturedSampleCount > 0) {
        self->captureElapsed += delta;
        if (kFloat_30_PadSwing * self->captureElapsed >= kMinimumCaptureFrames_Swing) self->captureState = kCaptureTriggered;
        if (self->captureFrameLimit == 0) return self->captureState;
        if (kFloat_30_PadSwing * self->captureElapsed >= (float)(self->captureFrameLimit + kCaptureTimeoutGraceFrames)) {
            ((UpdateVirtual*)self)->v24(self->output, self->samples, self->capturedSampleCount);
            ((UpdateVirtual*)self)->v60(self->output, self->capturedSampleCount);
            self->captureState = kCaptureComplete;
            Vec3_SetAll(&self->captureEndAcceleration, kFloat_0_0_SwingAcceleration);
        }
    }
    return self->captureState;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Track five-sample Y trough and Z peak until Y recovers beyond the existing recovery ratio.
static inline void FindSwingRecoveryWindow(Vec3Sample* v, int count, int& i, int& recoveryEndpoint, 
    float& zWindowSum, float& yWindowSum, float& maximumZWindowSum, float& minimumYWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i) {
        zWindowSum += v[i].z;
        yWindowSum += v[i].y;
    }
    maximumZWindowSum = zWindowSum;
    minimumYWindowSum = yWindowSum;
    for (recoveryEndpoint = kPeakWindowSamples; recoveryEndpoint < count; ++recoveryEndpoint) {
        zWindowSum -= v[recoveryEndpoint - kPeakWindowSamples].z;
        zWindowSum += v[recoveryEndpoint].z;
        if (zWindowSum > maximumZWindowSum) maximumZWindowSum = zWindowSum;
        yWindowSum -= v[recoveryEndpoint - kPeakWindowSamples].y;
        yWindowSum += v[recoveryEndpoint].y;
        if (yWindowSum < minimumYWindowSum) minimumYWindowSum = yWindowSum;
        if (minimumYWindowSum < kFloat_neg0_5_SwingAccelerometer && yWindowSum > kFloat_0_8_SwingAnalysis * minimumYWindowSum) break;
    }

}

// Choose the strongest five-sample weighted Y/Z window before recovery.
static inline void SelectSwingCombinedPeak(Vec3Sample* v, int recoveryEndpoint, float yToZPeakRatio, int& i, 
    int& selectedEndpoint, float& combinedWindowSum, float& bestCombinedWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i) combinedWindowSum += yToZPeakRatio * v[i].y + v[i].z;
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
        if (selectedEndpoint < kDirectionWindowSamples) selectedEndpoint = kDirectionWindowSamples - 1;
        for (i = 0; i < kDirectionWindowSamples; ++i) {
            directionXSum += v[selectedEndpoint - i].x;
            directionYSum += v[selectedEndpoint - i].y;
        }
    }

}

// Store the mean magnitude of the five samples ending at the final selected endpoint.
static inline void MeasureSwingPeakStrength(SwingAnalysisView* self, Vec3Sample* v, int& i, float& peakMagnitudeSum) {
    for (i = 0; i < kPeakWindowSamples; ++i) peakMagnitudeSum += Vec3_Length(&v[self->peakSampleIndex - i]);
    self->peakMeanMagnitude = peakMagnitudeSum / kPeakWindowDivisor_Swing;

}

// Count overlapping Z windows above half the magnitude sum across the selected endpoint.
static inline void MeasureSwingPeakWidth(SwingAnalysisView* self, Vec3Sample* v, int count, 
    float peakMagnitudeSum, int& i, int& reachedPeakEndpoint, int& sustainedWindowCount, 
    float& widthZWindowSum) {
    for (i = 0; i < kPeakWindowSamples; ++i) widthZWindowSum += v[i].z;
    reachedPeakEndpoint = 0;
    sustainedWindowCount = 0;
    for (i = kPeakWindowSamples; i < count; ++i) {
        if (i == self->peakSampleIndex) reachedPeakEndpoint = 1;
        widthZWindowSum -= v[i - kPeakWindowSamples].z;
        widthZWindowSum += v[i].z;
        if (widthZWindowSum > 0.5f * peakMagnitudeSum) {
            ++sustainedWindowCount;
        } else {
            if (reachedPeakEndpoint != 0) break;
            sustainedWindowCount = 0;
        }
    }
    self->peakWidthSamples = (float)sustainedWindowCount;
}

// Analyze borrowed acceleration samples into direction, peak strength and sustained width.
// Requires five samples; returns 0 without outputs for shorter input, otherwise 1.
// End-to-end example: ten vectors (0,-1,1), mirror disabled. Pass 1 reaches
// count=10 with minY=-5,maxZ=5; pass 2 uses ratio=-1 and ties at endpoint 4.
// Pass 3 uses all ten samples and overrides the endpoint to 9: X=0,Y=-10,
// direction=atan(0/-10), numerically zero. Pass 4 yields mean magnitude sqrt(2).
// Pass 5 sees Z sum 5 > 0.5*(5*sqrt(2)), so endpoints 5..9 give width 5.
// The seed window at endpoint 4 is excluded from that width. These are empirical
// sample-window/threshold policies; no physical calibration law is recovered.
int CPadSwingAccele_AnalyzeCapturedSamples(SwingAnalysisView* self, Vec3Sample* v, int count) {
    if (count < kPeakWindowSamples) return 0;

    int i, reachedPeakEndpoint, sustainedWindowCount, recoveryEndpoint;
    float minimumYWindowSum, directionXSum, directionYSum, maximumZWindowSum;
    float zWindowSum = kFloat_0_0_SwingAcceleration;
    float yWindowSum = zWindowSum;
    // Pass 1: track the strongest five-sample Z sum and most negative Y sum.
    // Stop after negative Y recovers above 80% of its minimum, once that minimum
    // crossed -0.5. recoveryEndpoint is excluded by the next window search.
    FindSwingRecoveryWindow(v, count, i, recoveryEndpoint, zWindowSum, yWindowSum, maximumZWindowSum, minimumYWindowSum);

    // Pass 2: balance the Y trough against the Z peak, then maximize the combined
    // signal (Y*ratio + Z) over five samples. For minY=-10,maxZ=5 the ratio is
    // -0.5: a negative Y impulse contributes positively. Clamp minY away from zero.
    if (minimumYWindowSum > kFloat_neg0_001_PadSwingAcceleration) minimumYWindowSum = kFloat_neg0_001_PadSwingAcceleration;
    float yToZPeakRatio = maximumZWindowSum / minimumYWindowSum;

    int selectedEndpoint = kPeakWindowSamples - 1;
    float bestCombinedWindowSum;
    float combinedWindowSum = kFloat_0_0_SwingAcceleration;
    SelectSwingCombinedPeak(v, recoveryEndpoint, yToZPeakRatio, i, selectedEndpoint, combinedWindowSum, bestCombinedWindowSum);

    // Pass 3: direction uses at most 40 samples ending at the selected endpoint.
    // Short captures use ALL samples and move the endpoint to their last sample;
    // long captures clamp it to >=39 so the backwards sum cannot underflow.
    // This override also determines the five samples used for strength below.
    directionXSum = kFloat_0_0_SwingAcceleration;
    directionYSum = directionXSum;
    SumSwingDirectionWindow(v, count, i, selectedEndpoint, directionXSum, directionYSum);

    // Bound Z before recomputing the direction ratio; constrain the ratio to <=-1.
    // This is a second ratio with different clamps, not the Pass 2 search weight.
    if (directionYSum > kSwingDirectionYSumUpperBound) directionYSum = kSwingDirectionYSumUpperBound;
    float directionYWeight = maximumZWindowSum; // This pass converts the peak sum into a ratio.
    if (directionYWeight >= kFloat_15_SwingAccelerometer) directionYWeight = kFloat_15_SwingAccelerometer;
    directionYWeight /= minimumYWindowSum;
    if (directionYWeight > kFloat_neg1_swingAccelerometer) directionYWeight = kFloat_neg1_swingAccelerometer;
    directionYSum *= -directionYWeight;
    float angle = (float)atan((double)(directionXSum / directionYSum));
    self->swingDirectionAngle = angle;
    if (self->mirrorXAxis != 0) self->swingDirectionAngle = angle * kFloat_neg1_swingAccelerometer;
    self->peakSampleIndex = selectedEndpoint;

    // Pass 4: strength is mean vector magnitude over the five ending samples.
    float peakMagnitudeSum = kFloat_0_0_SwingAcceleration;
    MeasureSwingPeakStrength(self, v, i, peakMagnitudeSum);

    // Pass 5: count consecutive five-sample Z windows above half that magnitude
    // SUM (not half the mean). Discard runs ending before the selected endpoint;
    // the first below-threshold window at/after it finishes the search. The seed
    // window ending at 4 is never counted or marked as having reached the peak.
    // Width is a count of overlapping window endpoints, not disjoint samples.
    float widthZWindowSum = kFloat_0_0_SwingAcceleration;
    MeasureSwingPeakWidth(self, v, count, peakMagnitudeSum, i, reachedPeakEndpoint, sustainedWindowCount, widthZWindowSum);

    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return mean vector magnitude around the detected swing peak.
float CPadSwingAccele_vfunc_12(SwingAnalysisView* self) {
    return self->peakMeanMagnitude;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return the detected peak width measured in samples.
float CPadSwingAccele_vfunc_13(SwingAnalysisView* self) {
    return self->peakWidthSamples;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Return the handedness-adjusted direction angle found by the capture analysis.
float CPadSwingAccele_vfunc_14(SwingAnalysisView* self) {
    return self->swingDirectionAngle;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Accumulate absolute corrected acceleration over the requested capture fraction.
int CPadSwingAccele_SumAbsCorrectedSamplePrefix(SwingPartialSumView* self, int* outCount, Vec3* values, float fraction) {
    Vec3_SetAll(values, kFloat_0_0_SwingAcceleration);
    int limit = self->capturedSampleCount;
    int count = ((ns1::SwingVirtual*)self)->count();
    if (fraction * (float)count < (float)limit) {

        limit = (int)(fraction * (float)((ns1::SwingVirtual*)self)->count());
    }
    float zero = kFloat_0_0_SwingAcceleration;
    for (int i = 0; i < limit; i++) {
        float value = AsCPadSwingAccele(self)->correctedSampleX;
        if (value > zero) values->x += value;
        else values->x -= value;
        value = AsCPadSwingAccele(self)->correctedSampleY;
        if (value > zero) values->z += value;
        else values->z -= value;
        value = AsCPadSwingAccele(self)->correctedSampleZ;
        if (value > zero) values->y += value;
        else values->y -= value;
        self = (SwingPartialSumView*)((char*)self + sizeof(Vec3));
    }
    *outCount = limit;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Accumulate absolute corrected acceleration per axis and return at least one as the divisor count.
int CPadSwingAccele_SumAbsoluteCorrectedSamples(SwingAbsoluteSumView* self, float* outX, float* outY, float* outZ, int* outCount) { float accX = kFloat_0_0_SwingAcceleration; float accY = kFloat_0_0_SwingAcceleration; float accZ = kFloat_0_0_SwingAcceleration;

    SwingAbsoluteSumView* ptr = self;

    int count = self->capturedSampleCount;

    for (int i = 0; i < count; i++) {
        float value = ptr->correctedSampleX;
        if (value > (kFloat_0_0_SwingAcceleration)) accX += value;
        else accX -= value;
        value = ptr->correctedSampleZ;
        if (value > (kFloat_0_0_SwingAcceleration)) accY += value;
        else accY -= value;
        value = ptr->correctedSampleY;
        if (value > (kFloat_0_0_SwingAcceleration)) accZ += value;
        else accZ -= value;
        ptr = (SwingAbsoluteSumView*)((char*)ptr + sizeof(Vec3));
    }

    *outX = accX; *outY = accY;
    *outZ = accZ;
    count = self->capturedSampleCount;
    if (!count) *outCount = 1;
    else *outCount = count;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Copy the stored start/end capture vectors to two required output buffers.
int CPadSwingAccele_CopyCaptureAccelerations(void* self, const char* first, const char* second) {
    if (first == 0) return 0;
    if (second == 0) return 0;
    Vector_CopyComponents(first, (char*)AsCPadSwingAccele(self)->captureStartAcceleration);
    Vector_CopyComponents(second, (char*)AsCPadSwingAccele(self)->captureEndAcceleration);
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Return the number of captured swing samples.
unsigned int CPadSwingAccele_vfunc_15(SwingSampleCountView* self) {
    return self->capturedSampleCount;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Return the timing-mode-dependent resampler output sample count.
int CPadSwingAccele_GetResampleOutputSampleCount() {
    if (EventTiming_IsDurationScalingEnabled() != 0) {
        return 0xc;
    }
    return 0xa;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Remove held/stationary acceleration bias, clamp axes, and derive the current swing angle.
int CPadSwingAccele_CorrectAccelAndUpdateAngle(PadAccele* self, int enabled, float delta) {
    Vec3 value;
    float threshold, total;
    Vector_CopyComponents((const char*)&value, (char*)&self->current);
    if (enabled) {
        // Suppress recent history X during negative-axis holds, and clear all
        // history X after steady magnitude persists for the reset interval.
        total = kFloat_0_0_SwingAcceleration;
        if (value.y < total) self->negativeYHoldTime = kNegativeYHoldSeconds_Swing;
        float zeroY, tY;
        tY = self->negativeYHoldTime;
        zeroY = kFloat_0_0_SwingAcceleration;
        if (tY > zeroY) {
            for (int i = 0; i < self->historyCount; ++i) {
                int n = (self->head + kLastLongHistorySlot - i) % kLongHistorySamples;
                self->history[n].x = zeroY;
            }
            self->negativeYHoldTime -= delta;
        }
        if (value.x < kFloat_neg0_5_SwingAccelerometer) self->negativeXHoldTime = kFloat_0_5_SwingAccelerometer;
        float zeroX, tX;
        tX = self->negativeXHoldTime;
        zeroX = kFloat_0_0_SwingAcceleration;
        if (tX > zeroX) {
            for (int i = 0; i < self->historyCount; ++i) {
                int n = (self->head + kLastLongHistorySlot - i) % kLongHistorySamples;
                self->history[n].x = zeroX;
            }
            self->negativeXHoldTime -= delta;
        }
        float length = Vec3_Length(&value);
        if (length < kSteadyMagnitudeUpper_Swing && length > kSteadyMagnitudeLower_Swing) {
            float limit, s0;
            s0 = self->steadyAccelerationTime;
            limit = kSteadyResetSeconds_Swing;
            float s = s0 + delta;
            self->steadyAccelerationTime = s;
            if (s >= limit) {
                float zero = kFloat_0_0_SwingAcceleration;
                for (int j = 0; j < kLongHistorySamples; ++j) self->history[j].x = zero;
                self->steadyAccelerationTime = kFloat_0_0_SwingAcceleration;
            }
        } else self->steadyAccelerationTime = kFloat_0_0_SwingAcceleration;

        // Ring head is the next write: head+20..head+119 excludes the oldest
        // twenty slots. Sum only samples above the magnitude threshold, but
        // divide by the fixed bias divisor, not by the number accepted.
        threshold = kHistoryMagnitudeThreshold_Swing;
        for (int i = kBiasHistorySkip; i < kLongHistorySamples; ++i) {
            Vec3* item = &self->history[(self->head + i) % kLongHistorySamples];
            if (Vec3_Length(item) > threshold) total += self->history[(self->head + i) % kLongHistorySamples].x;
        }
        // This is cross-axis compensation: historical X lowers present Y.
        // Example: accepted X values sum to 10; the current divisor is 100,
        // so Y=0.6 becomes 0.5. A negative sum leaves Y unchanged. Halving the
        // divisor doubles this subtraction, including when few samples pass.
        // No sensor calibration derivation is established here. Tuning D affects
        // both the angle and any subsequent range decision, not the captured data.
        float average = total / kHistoryBiasSampleDivisor_Swing;
        if (average > kFloat_0_0_SwingAcceleration) value.y -= average;
    }
    // Clamp the local vector to [-1,1], then apply the original negative-Z correction.
    if (value.x > kFloat_1_SwingCapture) value.x = kFloat_1_SwingCapture;
    else if (value.x < kFloat_neg1_swingAccelerometer) value.x = kFloat_neg1_swingAccelerometer;
    if (value.y > kFloat_1_SwingCapture) value.y = kFloat_1_SwingCapture;
    else if (value.y < kFloat_neg1_swingAccelerometer) value.y = kFloat_neg1_swingAccelerometer;
    if (value.z > kFloat_1_SwingCapture) value.z = kFloat_1_SwingCapture;
    else if (value.z < kFloat_neg1_swingAccelerometer) value.z = kFloat_neg1_swingAccelerometer;
    // Current threshold/scale are -0.8 and 1.5: at Z=-1 this ADDS 0.3 to X.
    // There is no second clamp: X=0.9 can become 1.2 before angle projection.
    if (value.z < kNegativeZCorrectionThreshold_Swing) value.x = value.x - kNegativeZCorrectionScale_Swing * (kFloat_0_8_SwingAnalysis + value.z);

    // Piecewise angle in radians, measured from +Y toward +X in the first branch.
    // For X>0: acos(Y/hypot(X,Y)); e.g. (1,0,0) -> pi/2. For X<=0,Y<=0,
    // compare -X with Z to choose the XY or YZ projection. The XY branch uses
    // pi+sqrt(-X/hypot(X,Y)): (-1,0,0) -> pi+1, NOT 3*pi/2. The YZ branch
    // uses pi+asin(Z/hypot(Y,Z)): (0,-1,0) -> pi. Remaining X<=0,Y>0 uses
    // 3*pi/2+asin(Y/hypot(X,Y)): (0,1,0) -> 2*pi. These examples use the
    // already corrected/clamped vector. The unusual sqrt is empirical code,
    // not an established circular-angle identity; replacing it changes behavior.
    // A projection shorter than 0.001 preserves the previous angle.
    if (value.x > kFloat_0_0_SwingAcceleration) {
        float length = Math_SquareRootApprox(value.x * value.x + value.y * value.y);
        if (!(length < kFloat_0_001_PadSwing)) self->angle = acos(value.y / length);
    } else if (value.x <= kFloat_0_0_SwingAcceleration && value.y <= kFloat_0_0_SwingAcceleration) {
        if (-value.x > value.z) {
            float length = Math_SquareRootApprox(value.x * value.x + value.y * value.y);
            if (!(length < kFloat_0_001_PadSwing)) self->angle = kFloat_3_141592_PadSwing + Math_SquareRootApprox(-value.x / length);
        } else {
            float length = Math_SquareRootApprox(value.y * value.y + value.z * value.z);
            if (!(length < kFloat_0_001_PadSwing)) self->angle = kFloat_3_141592_PadSwing + (float)asin(value.z / length);
        }
    } else {
        float length = Math_SquareRootApprox(value.x * value.x + value.y * value.y);
        if (!(length < kFloat_0_001_PadSwing)) self->angle = kThreeHalfPi_Swing + (float)asin(value.y / length);
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Test current acceleration against the configured X/Y range and store the result.
int CPadSwingAccele_UpdateAccelerationInRangeFlag(SwingRangeQueryView* self) {
    char buffer[8];
    Vector_CopyComponents((const char*)buffer, (char*)AsCPadSwingAccele(self)->currentAcceleration);
    float _limit = kReadyAccelerationMinY_Swing;
    AsCPadSwingAccele(self)->accelerationInRange = 0;
    float y = *(float*)(buffer + 4);
    if (y >= _limit) {
        float x = *(float*)buffer;
        if (x >= kReadyAccelerationMinX_Swing && x <= kReadyAccelerationMaxX_Swing)
            AsCPadSwingAccele(self)->accelerationInRange = 1;
    }
    return AsCPadSwingAccele(self)->accelerationInRange;
}
}
#pragma pop

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
        Vector_CopyComponents((const char*)((char*)values + (kSampleBatchCapacity - 1) * sizeof(Vec3)), (char*)self + SWING_OFFSET(lastResampledVector));
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

// Resample one batch under the declaration's capacity/count contract, retaining the next endpoint.
int CPadSwingAccele_ResampleNewestFirstBatch(void* self, Vec3* output, Vec3* values, int inputSampleCount) {
    if (ResampleOutputSampleCount(self) <= 0)
        return 0;

    if (inputSampleCount == 0) {
        // No input: repeat the previous endpoint, leaving cache and source intact.
        Vec3* out = output;
        int i;
        FillResampleOutputFromCache(self, out, i);
        return 0;
    }

    // Append an older endpoint; a full raw batch sacrifices its last sample.
    AppendResampleCachedEndpoint(self, values, inputSampleCount);

    // Ratio is output samples per input interval, not seconds per sample.
    float t = (float)ResampleOutputSampleCount(self) / (float)inputSampleCount;
    Vec3* out = output;
    int outIndex, lower;
    InterpolateResampleOutput(self, values, t, out, outIndex, lower);
    Vector_CopyComponents((char*)self + SWING_OFFSET(lastResampledVector), (char*)values);
    return 1;
}
}
#pragma pop

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

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Initialize the motion state, timing, interface, and five owned channel pointers.
void CPadSwingCtrlMotion_Ctor(SwingMotionInitializationView* self) {
    float zero = kFloat_0_805E0078;
    self->motionState = 0;
    int addr = (int)__vt__19CPadSwingCtrlMotion;
    self->motionVtable = addr;
    self->motionPosition = zero;
    self->motionVelocity = zero;
    self->motionAcceleration = zero;
    self->motionStep = 0.1f;
    self->motionSettled = 0;
    self->animationStarted = 0;
    self->animationElapsed = zero;
    self->settlingElapsed = zero;
    self->completionElapsed = zero;
    self->motionLimit = zero;
    self->ownedChannel0 = 0;
    self->ownedChannel1 = 0;
    self->ownedChannel2 = 0;
    self->ownedChannel3 = 0;
    self->ownedChannel4 = 0;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
#undef PROCESS_AXIS
extern "C" {
// Delete all five owned motion-channel arrays and optionally release the controller.
void* CPadSwingCtrlMotion_dtor(void* object, int shouldDelete) {
    if (object != 0) {
        AsCPadSwingCtrlMotion(object)->motionVtable = (void*)__vt__19CPadSwingCtrlMotion;
        char* ptr = (char*)object;
        int zero = 0;
        int i = 0;
        for (; i < 5; i++) {
            if (AsCPadSwingCtrlMotion(ptr)->firstOwnedMotionChannel != 0) {
                __dla__FPv(AsCPadSwingCtrlMotion(ptr)->firstOwnedMotionChannel);
                AsCPadSwingCtrlMotion(ptr)->firstOwnedMotionChannel = (void*)zero;
            }
            ptr += 4;
        }
        if (shouldDelete > 0)
            __dl__FPv(object);
    }
    return object;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Return the current motion-state value.
unsigned int CPadSwingCtrlMotion_vfunc_1(unsigned int* self) {
    return *self;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Advance the motion phase, clearing the timing and position data needed by its next phase.
int CPadSwingCtrlMotion_AdvanceMotionPhase(SwingMotionStateView* self) {
    int changed = 0;
    switch (self->motionState) {
    // Start a return cycle with zero position, velocity and acceleration.
    case kMotionIdle: {
        float zero = kFloat_0_805E0078;
        self->motionState = kMotionReturning;
        changed = 1;
        self->motionPosition = zero;
        self->motionVelocity = zero;
        self->motionAcceleration = zero;
        break;
    }
    // Advance the return phase to forward motion.
    case kMotionReturning:
        self->motionState = kMotionForward;
        changed = 1;
        break;
    // Reset animation/settling timers before settling.
    case kMotionForward: {
        float zero = kFloat_0_805E0078;
        self->motionSettled = 0;
        changed = 1;
        self->animationStarted = 0;
        self->animationElapsed = zero;
        self->settlingElapsed = zero;
        self->motionState = kMotionSettling;
        break;
    }
    // Mark completion and clear completion timing/position.
    case kMotionSettling: {
        float zero = kFloat_0_805E0078;
        self->motionState = kMotionComplete;
        changed = 1;
        self->motionPosition = zero;
        self->completionElapsed = zero;
        break;
    }
    // Return to idle; no new motion-change flag.
    case kMotionComplete:
        self->motionState = kMotionIdle;
        break;
    }
    return changed;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Cancel forward motion by switching to the return phase; report whether a change occurred.
int CPadSwingCtrlMotion_CancelForwardMotion(int* self) {
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

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Return the motion controller to idle and clear its timing, position, and completion data.
int CPadSwingCtrlMotion_ResetMotionToIdle(SwingMotionStateView* self) {
    float f0 = kFloat_0_805E0078;
    self->motionState = kMotionIdle;
    self->motionPosition = f0;
    self->motionVelocity = f0;
    self->motionAcceleration = f0;
    self->motionSettled = 0;
    self->animationStarted = 0;
    self->animationElapsed = f0;
    self->settlingElapsed = f0;
    self->completionElapsed = f0;
    self->animationStartWaitElapsed = f0;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
#undef PROCESS_AXIS
extern "C" {
// Bind the motion entry selected by the virtual motion-index query to the supplied object.
float CPadSwingCtrlMotion_BindSelectedMotion(void* self, Object0* object) {
    ((PC0*)object)->vC0((char*)self + 8, swingMotionNames + ((P44*)self)->v60() * 0x24);
    return kFloat_0_805E0078;
}
}
#pragma pop

}
