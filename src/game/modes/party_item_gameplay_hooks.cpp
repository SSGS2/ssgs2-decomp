// Party Goods gameplay hooks: passive bonuses, motion gestures, wind/spin badges,
// timebomb and shared Stamp Hammer balloon selection/history. Factory/dispatch:
// PartyItem_Create, PartyItemSlots_RebuildEffectParameters and PartyItems_ApplySelection
// in party_goods_item_display.cpp; modifier schema: PartyItemModifiers.h.
// Mod lookup: launch height -> Feather constructor + CGolfRule::StartBallFlightFromSample;
// club-distance/impact bonuses -> modifier consumer map below; hammer cone ->
// kHammerTargetHalfConeRadians/SelectNearestBalloonInRange; release delay ->
// s_stampHammerReleaseDelayTicks; random club badge -> item-11 contract below.
// Constants are shared: e.g. kFloat_30_PartyBadge feeds Titan, Hurricane and Feather;
// changing its storage changes all three. A future independent setting must split
// the producer's use, not silently change the shared pool. No float rewrite here.
#pragma push
#include "classes/CBalloonSystem.h"
#include "classes/CGolfDoc.h"
#include "game/golfball.h"
#include "classes/CPartyItemFeatherBadge.h"
#include "classes/CPartyItemFireBantage.h"
#include "classes/CPartyItemFujinUchiwa.h"
#include "classes/CPartyItemHurricaneBadge.h"
#include "classes/CPartyItemLuckyBracelet.h"
#include "classes/CPartyItemPululuJarashi.h"
#include "classes/CPartyItemRakugakiCrayon.h"
#include "classes/CPartyItemRandomClub.h"
#include "classes/CPartyItemSilentMisanga.h"
#include "classes/CPartyItemStampHammer.h"
#include "classes/CPartyItemTimebombBadge.h"
#include "classes/CPartyItemTitanWristband.h"
#include "classes/CSharedDoc.h"
#include "classes/CWind.h"
#include "classes/PartyItemModifiers.h"
#include "classes/PartyRibbonState.h"

#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCPartyItemFeatherBadge(p) ((CPartyItemFeatherBadge*)(p))
#define AsCPartyItemFujinUchiwa(p) ((CPartyItemFujinUchiwa*)(p))
#define AsCPartyItemHurricaneBadge(p) ((CPartyItemHurricaneBadge*)(p))
#define AsCPartyItemLuckyBracelet(p) ((CPartyItemLuckyBracelet*)(p))
#define AsCPartyItemPululuJarashi(p) ((CPartyItemPululuJarashi*)(p))
#define AsCPartyItemRakugakiCrayon(p) ((CPartyItemRakugakiCrayon*)(p))
#define AsCPartyItemSilentMisanga(p) ((CPartyItemSilentMisanga*)(p))
#define AsCPartyItemStampHammer(p) ((CPartyItemStampHammer*)(p))
#define AsCPartyItemTimebombBadge(p) ((CPartyItemTimebombBadge*)(p))
#define AsCPartyItemTitanWristband(p) ((CPartyItemTitanWristband*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsCWind(p) ((CWind*)(p))
// View a borrowed modifier payload; offset zero is a numeric modifier, not a vtable.
inline PartyItemModifiers* AsPartyItemModifiers(void* p) {
    return (PartyItemModifiers*)p;
}
// View the crayon's borrowed ribbon state without transferring ownership.
inline PartyRibbonState* AsPartyRibbonState(void* p) {
    return (PartyRibbonState*)p;
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CFx.h"

// Construct in existing storage; the container remains responsible for that storage.
inline void* operator new(size_t, void* p) throw() {
    return p;
}
// Placement-delete hook: construction failure does not free caller-owned storage.
inline void operator delete(void*, void*) throw() {}

// Modifier offsets below are RELATIVE to PartyItemModifiers, which is manager+10
// in party_goods_item_display.cpp. Hooks borrow this shared block; it is not reset before every hook.
// Consumer map (quantities/rules, not interchangeable "bonus" multipliers):
// +00 clubDistanceBonusRate: dimensionless fractional gain. GolfParameter_ResolveFloat
//     (club_selection_and_shot_params.cpp) computes FMADDS(distanceYards,rate,distanceYards) in party mode 3,
//     excludes selected putter and inventory slot-14 special club set. Zero neutral.
// +04/+08 impact/Pangya half-width gains: fractional width gains, applied as 1+gain
//     by impact consumers (club_selection_and_shot_params.cpp/golf_rule_round_flow_and_shot_sim.cpp). Silent defaults 1 / 0.5 respectively.
// +0C gaugeSpeedMultiplier: nonzero scales button gauge advance (golf_rule_round_flow_and_shot_sim.cpp);
//     zero bypasses scaling, Silent default 0.7 slows gauge advance.
// +10 positiveShotOutcomeBonusRate: (1+rate) scales only positive outcome delta
//     in GolfRule_ApplyShotOutcomeModifier, not Pang or negative outcomes. Default 1.
// +14 launchVerticalVelocityBonus: (1+bonus) scales launch/replay vy; Feather default 1.
// +18/+40/+44 Hurricane wind magnitude/force-unit/override gates (wind_system.cpp/wind_regions_and_vectors.cpp).
// +1C/+20 fan speed/heading accompany separate additive CWind XYZ writes.
// +24/+28/+48 spin-panel Y/X radii and override flag (CSpin_OnProcess, tail_effect_spin_and_bubble.cpp);
//     Feather initializes radii to 30 in panel units; this is not a spin rate.
// +2C impactZoneWidthDivisor: positive divisor (tickle publishes >=1); never zero.
// +30 ballScaleBonus: display scale uses 1+bonus (CBall_OnDisplay, ball_actor_and_ambient_sound.cpp).
//     Collision radius is set separately by Titan; +34 is cleared but no consumer
//     is established. It is not evidence of another tuning effect.
// +38/+3C/+50 timebomb: elapsed calls, period (period*18 tick bound), trigger;
//     trigger affects clock/swing/HUD, not a wall-clock seconds value.
// +54 tickle activity drives HUD scripts. Safe tuning bounds beyond these observed
//     guards are unestablished; do not reinterpret quantities with different units.
// Party item kinds are separate from pangya.iff type IDs (whose kind is typeId >> 26).
// The pre-shot hook prepares per-shot modifiers; the apply hook updates or applies an item for the
// supplied player index. Return 1 generally includes "no effect this call".
// Modifier/ribbon pointers are borrowed. Gesture cooldowns, tickle decay and
// timebomb elapsedTicks advance per invocation; these functions receive no dt.
enum PartyItemKind {
    kSilentMisanga = 1,
    kLuckyBracelet = 2,
    kTitanWristband = 3,
    kFujinUchiwa = 4,
    kRakugakiCrayon = 5,
    kPululuJarashi = 6,
    kStampHammer = 7,
    kHurricaneBadge = 8,
    kFeatherBadge = 9,
    kTimebombBadge = 10,
    kPartyItem11 = 11
};
// Shared-player +0x1DC is the button-swing selection word; zero selects motion.
// Evidence: CGolfRule enters MotionSwing when it is zero (golf_rule_round_flow_and_shot_sim.cpp).
enum PartyControlMode {
    kPartyMotionControls = 0
};
enum PartyTimebombShotPhase {
    kTimebombPhase0 = 0,
    kTimebombPhase2 = 2
};
enum PartyItemUseMode {
    kPassiveItem = 0,
    kGestureItem = 1,
    kBadgeItem = 2
};
enum PartyItemInput {
    kActionUsePartyItem = 0x15, // logical input action, not a Wii hardware button mask
    kFanGesture = 0,
    kTickleGesture = 1,
    kHammerGesture = 2,
    kGestureRumbleTicks = 3
};
enum PartyItemTiming {
    kFanEffectCooldownTicks = 45,
    kTitanRampLength = 5,
    kTimebombTicksPerPeriod = 18,
    kDefaultTimebombPeriod = 30,
    kRibbonPointCount = 20
};
// Values/roles verified against CGolfRule::OnProcess/EnterState in golf_rule_round_flow_and_shot_sim.cpp.
// Item gestures admit aiming, camera view/transition, and motion swing. Tickle also
// admits button swing. Timebomb shot-state admission uses aiming/motion swing;
// camera override and post-trigger continuation are separate (see EvaluateTimebombAdmission).
enum PartyInputGolfState {
    kPartyStateAim = 0x8,
    kPartyStateButtonSwing = 0x20,
    kPartyStateCameraView = 0x100,
    kPartyStateCameraTransition = 0x200,
    kPartyStateMotionSwing = 0x08000000
};
enum PartyItemStateMask {
    kPartyItemInputStates = kPartyStateAim | kPartyStateCameraView | kPartyStateCameraTransition |
        kPartyStateMotionSwing,
    kTickleAdditionalState = kPartyStateButtonSwing,
    kTimebombCountStates = kPartyStateAim | kPartyStateMotionSwing
};
enum PartyEffectStringOffsets {
    kHugeBallSound = 0,
    kFanSoundAlias = 0x18,
    kFanEffectSequence = 0x23,
    kFanWindSound = 0x36,
    kCrayonSoundAlias = 0x4B,
    kCrayonDrawSound = 0x52,
    kTickleSound = 0x6A
};
enum BalloonDequeLayout {
    kPointersPerDequeBlock = 256,
    kDequeBlockShift = 8,
    kDequeBlockMask = 255,
    kDequeBlockBytes = 0x400,
    kDequeTrimThreshold = 512,
    kDequeInitialOffset = 128
};

// Gesture result consumed by the fan: steering and magnitude are separate floats.
struct FanGestureResult {
    char prefix[0x34];
    float steering;
    float magnitude;
};
struct WindAdjustmentView {
    char prefix[4];
    float baseAdjustment[3];
    float additiveAdjustment[3];
};
struct CrayonRibbonActivity {
    char prefix[0x4C];
    int drawing;
};
struct BalloonListNode {
    void* previous;
    void* next;
    unsigned int balloon;
};
struct BalloonListView {
    unsigned int count;
    void* sentinel;
    void* first;
};
// Four shared-document player records begin after its 0x14-byte prefix.
// The control-mode word is 0x1C8 bytes into each 0x1D4-byte record.
struct SharedPlayerControlRecord {
    char prefix[0x1C8];
    int buttonSwingSelected;
    char tail[8];
};
struct SharedPlayerControls {
    char prefix[0x14];
    SharedPlayerControlRecord players[4];
};
typedef char PlayerControlStrideCheck[sizeof(SharedPlayerControlRecord) == 0x1D4 ? 1 : -1];
struct PartyInstanceSettings {
    char prefix[0xA];
    unsigned char mode;
};
struct PartyInstance {
    char prefix[4];
    PartyInstanceSettings* settings;
};
struct OwnedContainerBuffer {
    void* data;
    unsigned int allocator;
    int count;
};
#define AsFanGestureResult(p) ((FanGestureResult*)(p))
#define AsWindAdjustment(p) ((WindAdjustmentView*)(p))
#define AsBalloonListNode(p) ((BalloonListNode*)(p))
#define AsBalloonList(p) ((BalloonListView*)(p))
#define AsPhysicsBall(p) ((CGolfBall*)(p))

// ---- merged from party_item_gameplay_hooks.cpp ----
namespace ns0 {

extern "C" {

// Fan pre-shot hook; no modifier reset is required.
void party_CPartyItemFujinUchiwa_vfunc_3();

// Crayon pre-shot hook; ribbon state is updated separately.
void party_CPartyItemRakugakiCrayon_vfunc_3();

// Reset the tickle item active-tick counter.
void party_CPartyItemPululuJarashi_vfunc_3(void* p);

// Copy Fire Bantage numeric payload + impact-zone width bonus for the owner only.

// Copy Pangya-zone width bonus and unresolved Lucky Bracelet payload for the owner only.

// Apply the active owner's Fire Bantage payload and impact-zone width bonus.

// Apply the active owner's Pangya-zone width bonus and unresolved Lucky Bracelet payload.

// Apply the three Silent Misanga owner modifiers while active.
int party_CPartyItemSilentMisanga_vfunc_4(void* self, void* /*unused*/, void* modifiers, int playerIndex);

void TrailRibbon_Build(void* p);

void PartyIItem_ctor(void* self, void* arg, int kind, int flag);
float BalloonCollision_ResetBallRadius();
float BalloonCollision_GetBallRadius();
void party_IItem_dtor(void* self, int flag);
void __dl__FPv(void* self);
extern char __vt__Q25party23CPartyItemPululuJarashi[];
extern char __vt__Q25party21CPartyItemStampHammer[];
extern char __vt__Q25party23CPartyItemLuckyBracelet[];
extern const float kFloat_0_PartyItem;
extern const float kFloat_5_TitanBase;        // 5.0: initial Titan base value
extern const float kFloat_30_PartyBadge;      // 30.0: Titan ramp modifier / badge parameter
extern const float kFloat_4_TitanRadiusRatio; // 4.0: Titan collision-radius increase ratio
extern const float kFloat_0_5_PartyEffects;   // 0.5: party effect sound volume / crayon opacity
extern const float kFloat_0_7_PartyMisanga;   // 0.7: Silent Misanga secondary modifier
extern char __vt__Q25party24CPartyItemTitanWristband[];
extern char __vt__Q25party23CPartyItemSilentMisanga[];
extern const float kFloat_1_PartyItem;
extern CSharedDoc* g_sharedDoc;
extern char __vt__Q25party21CPartyItemFujinUchiwa[];
extern char partyGoodsEffectStrings[];
extern void* g_soundManager;
void FxAttachment_Init(void* self, const char* name);
void __as__7WVectorFRC7WVector(void* self, void* other);
extern "C" void GetWindVector__5CWindFv(void* result, const void* self);
void Sound_StopByAlias(void* manager, const char* name);
void __dla__FPv(void* ptr);
void* __nw__FUl(unsigned int size);
void TrailRibbon_Ctor(void* self, void* arg, int a, int b);
void TrailRibbon_SetStyle(void* self, void* descriptor);
void TrailRibbon_Update(void* self);
void* InputManager_Init(void* self);
void InputManager_Dtor();
void __register_global_object(void* object, void* destructor, void* control);
void* GolfPlayer_GetEntryByIndex(void* self, int index);
int InputManager_GetPointerPosition(void* self, void* out, void* data);
int Input_IsActionButtonDown(void* self, int action, void* data);
int Input_IsActionPressed(void* self, int action, void* data);
void InputManager_StartRumble(void* self, int action);
void PartyItem_ResetIds(void* self);
int PartyItemMotion_DetectGesture(void* self, int value);
void TrailRibbon_SetPointerPosition(void* self, void* arg);
int SoundManager_IsAliasPrefixOrStreamStemActive(void* manager, const char* name);
void Sound_PlayByName(void* manager, const char* name, int a, int b, const char* other, int c, float minimumGain,
                      float attenuationRange, float volumeScale);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_partyItems[];
extern const char partyCrayonPlayerColors[];
extern char __vt__Q25party24CPartyItemRakugakiCrayon[];
extern const float kFloat_15_PartyEffects;   // 15.0: ribbon width / minimum Hurricane value
extern const float kFloat_200_PartyEffects;  // 200.0: party sound playback parameter
extern float g_tickleStrengthPerGesture;     // 0.1 strength gained per tickle gesture
extern float g_tickleStrengthDecayPerUpdate; // 0.02 strength lost per update
extern float s_fanSteeringDeadZone;
extern float g_fanGestureWindSpeedScale; // 0.32 fan gesture-to-wind-speed scale
extern float kFloat_0_01_FanHeading;
extern const float kFanGestureMinimumMagnitude; // 10.0: minimum fan gesture magnitude
extern const float kFanMaximumHeadingDeviation; // pi/2: maximum fan heading deviation from aim
extern const float kFanMaximumWindSpeed;        // 9.0: maximum fan wind speed

void* PartyItemSlots_GetInstance();
void* Object_GetSlotEntry(void* context, void* data);
void* Matrix_SetAxisRotation(void* out, float angle, int flag);
void* Matrix_TransformPoint(void* out, void* a, void* b);
void __amu__7WVectorFf(void* self, float scale);
void* __mi__FRC7WVectorRC7WVector(void* out, void* a, void* b);
int Input_IsActionButtonDown(void* self, int action, void* data);
void Sound_PlayByName(void* manager, const char* name, int a, int b, const char* other, int c, float minimumGain,
                      float attenuationRange, float volumeScale);
extern void* g_fx;
extern char s_cameraTransform[];

typedef Vec3f Vec3;
struct Vec3i {
    int x, y, z;
};

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
enum FanGestureBranch {
    kFanNoGesture = 0,
    kFanNeutralSteering = 1,
    kFanPositiveSteering = 2,
    kFanNegativeSteering = 3
};
// Codes identify the steering branch; only zero/nonzero is consumed below.
// Apply the owner fan gesture to opponents as bounded wind heading/speed and refresh its effects.
int CPartyItemFujinUchiwa_UpdateOpponentWind(void* self, void* unused, void* out, int index);
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Release every ribbon buffer, stop the crayon sound and destroy the item.
void* party_CPartyItemRakugakiCrayon_dtor(void* self, int deletingFlag);
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

// Feed held-action pointer positions to the crayon ribbon and play its drawing sound.
int CPartyItemRakugakiCrayon_UpdateDrawing(void* self, void* unused, void* unused2, int index);

// Create a 20-point ribbon using the owner color and initial pointer position.
void* CPartyItemRakugakiCrayon_Ctor(void* self, int index, void* resource);

// Restore the default ball size before destroying the Titan Wristband.
void* party_CPartyItemTitanWristband_dtor(void* self, int deletingFlag);

// Capture the starting wind vectors and aim heading; clear the attached effect handle.
int CPartyItemFujinUchiwa_InitWindAndHeading(register char* self);

// Stop the fan sound alias and destroy the fan item.
void* party_CPartyItemFujinUchiwa_dtor(void* self, int deletingFlag);

// Initialize an inactive Lucky Bracelet with both modifier values set to one.
void* CPartyItemLuckyBracelet_Ctor(void* self, void* arg);

// Initialize inactive Silent Misanga owner modifiers and its enable flag.
void* CPartyItemSilentMisanga_Ctor(void* self, void* arg);

// Initialize the ball-size effect with a five-step ramp at either flight boundary.
void* CPartyItemTitanWristband_Ctor(void* self, void* arg);

// Restore the base ball size and clear the owner Titan modifiers before a shot.
void party_CPartyItemTitanWristband_vfunc_3(void* self, void* destination, int index);

// Destroy the party-item base and optionally free the item allocation.
void* CPartyItemLuckyBracelet_dtor(void* self, int deletingFlag);

// Destroy the party-item base and optionally free the item allocation.
void* CPartyItemPululuJarashi_dtor(void* self, int deletingFlag);

// Initialize the gesture-driven tickle item with zero strength and active ticks.
void* CPartyItemPululuJarashi_Ctor(void* self, void* arg);

// Initialize the hammer item and clear its three state words.
void* CPartyItemStampHammer_Ctor(void* self, void* arg);

// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemSilentMisanga_dtor(void* self, int deletingFlag);

// Select Silent Misanga owner modifiers using the current player input-mode word.
void party_CPartyItemSilentMisanga_vfunc_3(void* a, void* dest, int index, int unused);

// Initialize the fan attachment, clear its effect handle and set a 45-tick effect cooldown.
void* CPartyItemFujinUchiwa_Ctor(void* self, void* arg);

// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemStampHammer_dtor(void* self, int deletingFlag);

#pragma opt_lifetimes off
// Charge tickle strength from gestures, decay it each tick and apply it to the current opponent.
int CPartyItemPululuJarashi_UpdateTickleStrength(void* self, void* unused, void* destination, int index);
#pragma opt_lifetimes reset

void* InstanceTable_Get(void*);
void* GolfBall__Fv();
void Balloon_SetBallCollisionRadius(float);
extern const double kDouble_1_TitanWristband;

// Ramp the owner ball-size effect across the first/last five event-index steps.
int party_CPartyItemTitanWristband_vfunc_4(void* self, void* unused, void* destination, int index);

// Rebuild the crayon ribbon geometry.
int CPartyItemRakugakiCrayon_DrawRibbon(void* a);

} // extern "C"

} // namespace ns0

// ---- merged from party_item_gameplay_hooks.cpp ----
namespace ns1 {

extern "C" {

extern char __vt__Q25party23CPartyItemTimebombBadge[];
extern char s_partyItem11Vtable[];
extern char __vt__Q25party22CPartyItemFeatherBadge[];
extern char __vt__Q25party24CPartyItemHurricaneBadge[];
extern void* g_stampHammerBalloonQueue;
extern int g_stampHammerTargetBalloon;
extern int s_stampHammerCooldownTicks;
// Stored s32 update-call count (default 20 at 805DB3E8), despite this legacy
// float declaration. Both consumers below deliberately load it through int*.
// Increasing delays history release; duplicate items can suppress release only.
extern float s_stampHammerReleaseDelayTicks;
extern const float kFloat_30_PartyBadge; // 30.0: Titan ramp modifier / badge parameter
extern const float kFloat_1_PartyItem;
extern const float kFloat_15_PartyEffects; // 15.0: ribbon width / minimum Hurricane value
extern const float kFloat_0_PartyItem;
extern const float kPi_PartyHammer;              // pi: hammer facing rotation
extern const float kNegativePi_PartyHammer;      // -pi: lower signed-angle bound
extern const float kTwoPi_PartyHammer;           // 2*pi: full-turn angle
extern const float kHammerTargetHalfConeRadians; // 0.52359879 rad (pi/6): increase to admit wider angles
// 805E03E8 is an eight-byte pool slot with leading float32, NOT a double.
// Both selection passes share it; the failed-seed fallback also uses its numeric
// value as a world-distance cutoff. Preserve that coupling until a separate mod.
extern const float kFloat_neg1_PartyEffects; // -1.0: sign/sentinel value
extern float s_hurricaneBadgeWindValue;
extern CSharedDoc* g_sharedDoc;
void* PartyIItem_ctor(void*, int, int, int);
void* __nw__FUl(unsigned int);
void party_IItem_dtor(void*, int);
void BlockedWordRing_dtor(void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
unsigned int time(int);
void srand(unsigned int);
int PartyItemSlots_GetInstance(void);
int Object_GetSlotEntry(int, int);
void InputManager_Dtor(void);
extern char s_inputManagerDtorRegistration_partyItems[];
int PartyItemSlots_GetItemType(int, int);
void* dtor_8025A638(void*, int);
void* OwnedContainerBuffer_dtor(void*, int);
int party_CPartyItemStampHammer_vfunc_1(void);
void PartyStampHammer_SelectNearestBalloonInRange(void*);
void* Balloon_GetUnhitBalloonQueue(void*);
void* Balloon_GetPosition(void*);
void* GolfBall__Fv(void);
void Vec3_Subtract(void*, void*, void*);
void Vec3_Normalize(void*, void*);
float Vec3_Dot(void*, void*);
float Vec3_Length(void*);
double acos(double);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void InputManager_Init(void*);
int Input_IsActionPressed(void*, int, int);
int Input_IsActionButtonDown(void*, int, int);
int PartyItem_ResetIds(int);
void Mtx44_SetRotationY(void*, float);
void Mtx44_TransformDirection(void*, void*, void*, float);
void Vec3_Normalize(void*, void*);
int PartyItemMotion_DetectGesture(int, int);
int BlockedWordRing_EnsureAdditionalCapacity(void*, int);
int Balloon_IsActive(void*);
short Balloon_GetCurrentScaleIndex(void*);

struct VObj {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
};

// Shared hammer history contains borrowed balloon pointers; it never deletes
// balloons. blocks owns a ring of 256-word pages, addressed relative to start.
// base is the logical first slot within those pages, count is live entries,
// blockCount includes reserved end space; capacity counts slots in the page ring.
// Absolute slot p maps to blocks[(start+(p>>8))%capacity][p&255].
// Destruction frees pages and the pointer ring, not their balloon payloads.
struct BalloonDeque {
    unsigned int base;
    unsigned int count;
    void** blocks;
    unsigned int start;
    unsigned int blockCount;
    unsigned int capacity;
};

struct AllocatorTag {
    char value;
    char padding[3];
};

struct HammerSelf {
    char prefix[0x10];
    float direction[3];
};
enum HammerDirectionLayout {
    kHammerDirectionFloatIndex = (int)&((HammerSelf*)0)->direction / sizeof(float)
};

struct HammerVector {
    float x, y, z;
    // Set the local hammer direction before transforming it into world space.
    void set(float a, float b, float c) {
        x = a;
        y = b;
        z = c;
    }
};

extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];

// Borrow the current golf document; the shared document must already exist.
inline void* PartyGolfDocument() {
    return (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
}

// Require the party-item action and a recognized hammer gesture.
inline int hammerHeld(int actor) {
    int result;
    do {
        if (Input_IsActionButtonDown(GetInputManagerSlot(), kActionUsePartyItem, actor) == 0) {
            result = 0;
            break;
        }
        result =
            PartyItemMotion_DetectGesture(Object_GetSlotEntry(PartyItemSlots_GetInstance(), actor), kHammerGesture);
    } while (0);
    return result;
}

// Return the deque block pointer, or null when the deque has no allocated blocks.
inline void** hammerBlock(BalloonDeque* q, void** blockp) {
    void** result;
    do {
        if (q->blockCount == 0) {
            result = 0;
            break;
        }
        result = (void**)*blockp;
    } while (0);
    return result;
}

// Hammer pre-shot hook; shared queue reset occurs in its initialization hook.
void party_CPartyItemStampHammer_vfunc_3(void);

// Release the shared hammer balloon deque and clear the selected target.
void CBalloonSystem_DestroySharedQueue(void);

// Advance the admitted timebomb clock and trigger after period times 18, unless the player wears this badge.
int party_CPartyItemTimebombBadge_UpdateTimebomb(void* self, int, void* output);

// Clear and release the first owned container buffer; optionally free the container.
void* dtor_8025A638(void* self, int shouldDelete);

// Clear and release the first owned container buffer; optionally free the container.
void* OwnedContainerBuffer_dtor(void* self, int shouldDelete);

// Reset the shared hammer cooldown, target and queued balloon count while retaining deque blocks.
int party_CPartyItemStampHammer_vfunc_1(void);

// Lazily allocate the shared six-word hammer balloon deque and clear its selected target.
void PartyStampHammer_EnsureSharedQueue(void);

// Hammer geometry hook; no extra geometry work is required.
int party_CPartyItemStampHammer_vfunc_5(void);

// Hammer finalization hook; no per-frame finalization is required.
void party_CPartyItemStampHammer_vfunc_6(void);

struct DequeIter {
    void** cur;
    void** begin;
    void** start;
    void** end;
    unsigned int* elem;
};

struct DequeRaw {
    void** cur;
    void** begin;
    void** start;
    void** end;
};

// Build a deque iterator for an absolute slot, retaining the original volatile iterator writes.
inline void dequeIterAt(volatile DequeIter* it, BalloonDeque* q, unsigned int pos) {
    DequeRaw raw;
    void** begin = q->blocks;
    void** endp = begin + q->capacity;
    void** startp = begin + q->start;
    ((volatile DequeRaw*)&raw)->begin = begin;
    ((volatile DequeRaw*)&raw)->cur = startp;
    ((volatile DequeRaw*)&raw)->start = startp;
    ((volatile DequeRaw*)&raw)->end = endp;
    int cap = endp - begin;
    int idx = (int)(pos >> 8) + (startp - begin);
    if (idx < 0)
        idx += cap;
    else if (idx >= cap)
        idx -= cap;
    void** blockp = ((volatile DequeRaw*)&raw)->begin + idx;
    void** rb = ((volatile DequeRaw*)&raw)->begin;
    void** rs = ((volatile DequeRaw*)&raw)->start;
    ((volatile DequeRaw*)&raw)->cur = blockp;
    void** re = ((volatile DequeRaw*)&raw)->end;
    unsigned int* base = q->blockCount == 0 ? 0 : (unsigned int*)*blockp;
    it->cur = blockp;
    it->begin = rb;
    it->start = rs;
    it->end = re;
    it->elem = base + (pos & 0xFF);
}

// Resolve an absolute deque slot to its element pointer.
inline unsigned int* dequeEndElem(BalloonDeque* q, unsigned int pos) {
    DequeRaw raw;
    void** begin = q->blocks;
    void** endp = begin + q->capacity;
    void** startp = begin + q->start;
    ((volatile DequeRaw*)&raw)->begin = begin;
    ((volatile DequeRaw*)&raw)->cur = startp;
    ((volatile DequeRaw*)&raw)->start = startp;
    ((volatile DequeRaw*)&raw)->end = endp;
    int cap = endp - begin;
    int idx = (int)(pos >> 8) + (startp - begin);
    if (idx < 0)
        idx += cap;
    else if (idx >= cap)
        idx -= cap;
    void** blockp = ((volatile DequeRaw*)&raw)->begin + idx;
    ((volatile DequeRaw*)&raw)->cur = blockp;
    unsigned int* base = q->blockCount == 0 ? 0 : (unsigned int*)*blockp;
    return base + (pos & 0xFF);
}

// Search the shared hammer deque for an already-queued balloon pointer.
inline bool dequeContains(const unsigned int* value) {
    BalloonDeque* q = (BalloonDeque*)g_stampHammerBalloonQueue;
    DequeIter it;
    DequeIter last;
    dequeIterAt(&it, q, q->base);
    q = (BalloonDeque*)g_stampHammerBalloonQueue;
    dequeIterAt(&last, q, q->count + q->base);
    DequeIter* v = &it;
    unsigned int* stop = last.elem;
    while (v->elem != stop && *v->elem != *value) {
        v->elem = v->elem + 1;
        if (v->elem == (unsigned int*)((char*)*v->cur + kDequeBlockBytes)) {
            v->cur = v->cur + 1;
            if (v->cur == v->end)
                v->cur = v->begin;
            v->elem = (unsigned int*)*v->cur;
        }
    }
    q = (BalloonDeque*)g_stampHammerBalloonQueue;
    unsigned int* found = v->elem;
    return found != dequeEndElem(q, q->count + q->base);
}

// Select the nearest unqueued balloon in the facing cone, seeding then refining a distance bound.
// Cone tests use horizontal world X/Z; candidate ranking uses full XYZ distance.
// No candidate leaves the previous global target unchanged. Queue/list payloads
// are borrowed; this function neither removes balloons nor adds history entries.
void PartyStampHammer_SelectNearestBalloonInRange(void* self);

// Return the container member at offset eight; its concrete owner remains unresolved.
void* Container_GetMember8(void* self);

// Hurricane update hook; its modifier is assigned by the pre-shot hook.
int party_CPartyItemHurricaneBadge_vfunc_4(void);

// Reset the elapsed timebomb ticks before a shot.
void party_CPartyItemTimebombBadge_vfunc_3(void* self);

// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemFeatherBadge_dtor(void* self, int shouldDelete);

// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemTimebombBadge_dtor(void* self, int shouldDelete);

// Copy owner spin overrides; publish the additive launch-velocity bonus for every player.

// While active, copy owner spin limits and every player's additive launch-velocity bonus.

// Clear the shared badge activation word (Feather active / item-11 applied latch).
void PartyItem_ClearActivationWord(void* self);

} // extern "C"

} // namespace ns1

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
namespace ns0 {
// Copy Fire Bantage numeric payload + impact-zone width bonus for the owner only.
void CPartyItemFireBantage::ApplyOwnerModifiers(void* modifiers, int playerIndex) {
    if (playerIndex != ownerIndex)
        return;
    // Fire Bantage +14 -> dimensionless club-distance rate at modifier +00;
    // see GolfParameter_ResolveFloat's party-mode/non-putter gates in club_selection_and_shot_params.cpp.
    // +04 independently widens good/poor impact half-widths, not club distance.
    AsPartyItemModifiers(modifiers)->clubDistanceBonusRate = clubDistanceBonusRate;
    AsPartyItemModifiers(modifiers)->impactZoneWidthBonus = impactZoneWidthBonus;
}
}
#pragma pop

#pragma push
namespace ns0 {
// Apply the active owner's Fire Bantage payload and impact-zone width bonus.
int CPartyItemFireBantage::ApplyModifiersIfActive(void* /*unused*/, void* modifiers, int playerIndex) {
    if (active != 0) {
        if (playerIndex == ownerIndex) {
            AsPartyItemModifiers(modifiers)->clubDistanceBonusRate = clubDistanceBonusRate;
            AsPartyItemModifiers(modifiers)->impactZoneWidthBonus = impactZoneWidthBonus;
        }
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize an inactive Silent Misanga and its three owner modifiers.
CPartyItemSilentMisanga::CPartyItemSilentMisanga(void* arg) {
    using namespace ns0;
    float v0;
    float v1;
    float v2;
    PartyIItem_ctor(this, arg, kSilentMisanga, kPassiveItem);
    v2 = kFloat_0_5_PartyEffects;
    vtbl = __vt__Q25party23CPartyItemSilentMisanga;
    v1 = kFloat_0_7_PartyMisanga;
    v0 = kFloat_1_PartyItem;
    active = 0;
    oneMeterWindEnabled = 1;
    pangyaZoneWidthBonus = v2;
    gaugeSpeedMultiplier = v1;
    impactZoneWidthBonus = v0;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemSilentMisanga_dtor(void* self, int deletingFlag) {
    if (self != 0) {
        party_IItem_dtor(self, 0);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Select Silent Misanga owner modifiers using the current player input-mode word.
void party_CPartyItemSilentMisanga_vfunc_3(void* a, void* dest, int index, int unused) {
    if (index != AsCPartyItemSilentMisanga(a)->ownerIndex)
        return;
    AsPartyItemModifiers(dest)->impactZoneWidthBonus = AsCPartyItemSilentMisanga(a)->impactZoneWidthBonus;
    void* manager = g_sharedDoc;
    void* owner = (*(void**)&AsCSharedDoc(manager)->golfDoc);
    unsigned char slot = AsCGolfDoc(owner)->currentPlayer;
    SharedPlayerControlRecord* entry = &((SharedPlayerControls*)manager)->players[slot];
    if (entry->buttonSwingSelected != kPartyMotionControls) {
        AsPartyItemModifiers(dest)->pangyaZoneWidthBonus = kFloat_0_PartyItem;
        AsPartyItemModifiers(dest)->gaugeSpeedMultiplier = AsCPartyItemSilentMisanga(a)->gaugeSpeedMultiplier;
    } else {
        float w = kFloat_1_PartyItem;
        float v = AsCPartyItemSilentMisanga(a)->pangyaZoneWidthBonus;
        AsPartyItemModifiers(dest)->pangyaZoneWidthBonus = v;
        AsPartyItemModifiers(dest)->gaugeSpeedMultiplier = w;
    }
    AsPartyItemModifiers(dest)->oneMeterWindEnabled = AsCPartyItemSilentMisanga(a)->oneMeterWindEnabled;
}
}
#pragma pop

#pragma push
namespace ns0 {
// Apply the three Silent Misanga owner modifiers while active.
int party_CPartyItemSilentMisanga_vfunc_4(void* self, void* /*unused*/, void* modifiers, int playerIndex) {
    if (AsCPartyItemSilentMisanga(self)->active != 0) {
        if (playerIndex == AsCPartyItemSilentMisanga(self)->ownerIndex) {
            AsPartyItemModifiers(modifiers)->pangyaZoneWidthBonus =
                AsCPartyItemSilentMisanga(self)->pangyaZoneWidthBonus;
            AsPartyItemModifiers(modifiers)->gaugeSpeedMultiplier =
                AsCPartyItemSilentMisanga(self)->gaugeSpeedMultiplier;
            AsPartyItemModifiers(modifiers)->impactZoneWidthBonus =
                AsCPartyItemSilentMisanga(self)->impactZoneWidthBonus;
        }
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize an inactive Lucky Bracelet with both modifier values set to one.
CPartyItemLuckyBracelet::CPartyItemLuckyBracelet(void* arg) {
    using namespace ns0;
    PartyIItem_ctor(this, arg, kLuckyBracelet, kPassiveItem);
    float value = kFloat_1_PartyItem;
    vtbl = __vt__Q25party23CPartyItemLuckyBracelet;
    active = 0;
    pangyaZoneWidthBonus = value;
    positiveShotOutcomeBonusRate = value;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Release the party-item base; the compiler adds the delete-if-requested wrapper.
CPartyItemLuckyBracelet::~CPartyItemLuckyBracelet() {
    using namespace ns0;
    party_IItem_dtor(this, 0);
}
#pragma pop

#pragma push
namespace ns0 {
// Copy Pangya-zone width bonus and unresolved Lucky Bracelet payload for the owner only.
void CPartyItemLuckyBracelet::ApplyOwnerModifiers(void* modifiers, int playerIndex) {
    if (playerIndex != ownerIndex)
        return;
    AsPartyItemModifiers(modifiers)->pangyaZoneWidthBonus = pangyaZoneWidthBonus;
    // Lucky Bracelet +18 -> modifier payload +10: fractional bonus applied only
    // to positive shot-outcome deltas (GolfRule_ApplyShotOutcomeModifier).
    AsPartyItemModifiers(modifiers)->positiveShotOutcomeBonusRate = positiveShotOutcomeBonusRate;
}
}
#pragma pop

#pragma push
namespace ns0 {
// Apply the active owner's Pangya-zone width bonus and unresolved Lucky Bracelet payload.
int CPartyItemLuckyBracelet::ApplyModifiersIfActive(void* /*unused*/, void* modifiers, int playerIndex) {
    if (active != 0) {
        if (playerIndex == ownerIndex) {
            AsPartyItemModifiers(modifiers)->pangyaZoneWidthBonus = pangyaZoneWidthBonus;
            AsPartyItemModifiers(modifiers)->positiveShotOutcomeBonusRate = positiveShotOutcomeBonusRate;
        }
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize the ball-size effect with a five-step ramp at either flight boundary.
CPartyItemTitanWristband::CPartyItemTitanWristband(void* arg) {
    using namespace ns0;
    PartyIItem_ctor(this, arg, kTitanWristband, kPassiveItem);
    float v0, v1, v2;
    v2 = kFloat_5_TitanBase;
    v1 = kFloat_30_PartyBadge;
    v0 = kFloat_4_TitanRadiusRatio;
    vtbl = __vt__Q25party24CPartyItemTitanWristband;
    baseValue = v2;
    maximumRampModifier = v1;
    collisionRadiusIncreaseRatio = v0;
    rampLength = kTitanRampLength;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Restore the default ball size before destroying the Titan Wristband.
void* party_CPartyItemTitanWristband_dtor(void* self, int deletingFlag) {
    if (self != 0) {
        AsCPartyItemTitanWristband(self)->vtbl = __vt__Q25party24CPartyItemTitanWristband;
        BalloonCollision_ResetBallRadius();
        party_IItem_dtor(self, 0);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Restore the base ball size and clear the owner Titan modifiers before a shot.
void party_CPartyItemTitanWristband_vfunc_3(void* self, void* destination, int index) {
    float value = BalloonCollision_ResetBallRadius();
    value = BalloonCollision_GetBallRadius();
    AsCPartyItemTitanWristband(self)->baseValue = value;
    if (index == AsCPartyItemTitanWristband(self)->ownerIndex) {
        float z = kFloat_0_PartyItem;
        AsPartyItemModifiers(destination)->ballScaleBonus = z;
        AsPartyItemModifiers(destination)->unk_34 = z;
    }
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Ramp the owner ball-size effect across the first/last five event-index steps.
int party_CPartyItemTitanWristband_vfunc_4(void* self, void* unused, void* destination, int index) {
    void* item = InstanceTable_Get(self);
    if (((PartyInstance*)item)->settings->mode == 3)
        return 1;
    if (AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] == -1)
        return 1;
    if (index == AsCPartyItemTitanWristband(self)->ownerIndex) {
        if (AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] == 0 ||
            AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] == -1) {
            BalloonCollision_ResetBallRadius();
            AsPartyItemModifiers(destination)->ballScaleBonus = kFloat_0_PartyItem;
        } else if (AsCPartyItemTitanWristband(self)->rampLength >=
                       AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] &&
                   AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] > 0) {
            if (AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] == 1)
                ((void (*)(void*, const char*, int, int, int, int, float, float, float))Sound_PlayByName)(
                    g_soundManager, partyGoodsEffectStrings, 0, 0, 0, 0, kFloat_0_5_PartyEffects,
                    kFloat_200_PartyEffects, kFloat_1_PartyItem);
            AsPartyItemModifiers(destination)->ballScaleBonus =
                AsCPartyItemTitanWristband(self)->maximumRampModifier /
                (float)AsCPartyItemTitanWristband(self)->rampLength *
                (float)AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0];
            float vy =
                (float)(AsCPartyItemTitanWristband(self)->baseValue *
                        (kDouble_1_TitanWristband + AsCPartyItemTitanWristband(self)->collisionRadiusIncreaseRatio /
                                                        (float)AsCPartyItemTitanWristband(self)->rampLength *
                                                        (float)AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0]));
            Balloon_SetBallCollisionRadius(vy);
        } else if (AsCPartyItemTitanWristband(self)->rampLength >=
                       AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] -
                           AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] &&
                   AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] -
                           AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] >
                       0) {
            AsPartyItemModifiers(destination)->ballScaleBonus =
                AsCPartyItemTitanWristband(self)->maximumRampModifier /
                (float)AsCPartyItemTitanWristband(self)->rampLength *
                (float)(AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] -
                        AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0]);
            float vz = (float)(AsCPartyItemTitanWristband(self)->baseValue *
                               (kDouble_1_TitanWristband +
                                AsCPartyItemTitanWristband(self)->collisionRadiusIncreaseRatio /
                                    (float)AsCPartyItemTitanWristband(self)->rampLength *
                                    (float)(AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] -
                                            AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0])));
            Balloon_SetBallCollisionRadius(vz);
        } else if (AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[1] -
                       AsPhysicsBall(GolfBall__Fv())->m_eventPreDataIndex[0] ==
                   0) {
            BalloonCollision_ResetBallRadius();
            AsPartyItemModifiers(destination)->ballScaleBonus = kFloat_0_PartyItem;
        }
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize the fan attachment, clear its effect handle and set the effect cooldown.
CPartyItemFujinUchiwa::CPartyItemFujinUchiwa(void* arg) {
    using namespace ns0;
    PartyIItem_ctor(this, arg, kFujinUchiwa, kGestureItem);
    vtbl = __vt__Q25party21CPartyItemFujinUchiwa;
    FxAttachment_Init(&effectAttachment, __vt__Q25party21CPartyItemFujinUchiwa);
    effectHandleValue = 0;
    cooldown = kFanEffectCooldownTicks;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Stop the fan sound alias and destroy the fan item.
void* party_CPartyItemFujinUchiwa_dtor(void* self, int deletingFlag) {
    if (self != 0) {
        AsCPartyItemFujinUchiwa(self)->vtbl = __vt__Q25party21CPartyItemFujinUchiwa;
        Sound_StopByAlias(g_soundManager, partyGoodsEffectStrings + kFanSoundAlias);
        party_IItem_dtor(self, 0);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
namespace ns0 {
// Fan pre-shot hook; no modifier reset is required.
void party_CPartyItemFujinUchiwa_vfunc_3() {}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Capture the starting wind and the aim heading; clear the attached effect handle.
int CPartyItemFujinUchiwa::InitWindAndHeading() {
    using namespace ns0;
    char local[0x10];
    GetWindVector__5CWindFv(local, CWind::GetInstance());
    __as__7WVectorFRC7WVector(&initialWind, local);
    __as__7WVectorFRC7WVector(&initialWindCopy, &initialWind);
    windHeading = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->headingRadians;
    effectHandleValue = 0;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
namespace ns0 {
// Resolve the fan's input singleton, constructing/registering it on first use.
static inline void EnsureFanInputManager() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_partyItems);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}

#pragma push
#pragma always_inline on
// Call the fan path zero-argument wind accessor ABI; the wider declaration is shared with other callers.
static inline void* GetFanWindSingleton() {
    return CWind::GetInstance();
}
#pragma pop

// Replace a weaker item wind while retaining the world's additive contribution.
// All scratch arrays remain caller-owned. Integer triples carry FLOAT BITS,
// never integer coordinates: vector helpers read/write the same 12 bytes. Save
// additive wind, temporarily zero it to measure the remaining wind, derive the
// base delta in world space, then restore those exact saved bits. speed uses
// the wind subsystem units (no conversion here); headings are radians.
static inline void ApplyFanWindModifier(CPartyItemFujinUchiwa* self, void*& out, float& speed, Vec3& desiredWind,
                                        Vec3i& savedAdditiveWindBits, float (&zeroAdditiveWind)[3],
                                        int (&unmodifiedWindBits)[3], int (&baseWindDeltaBits)[3], void*& wind,
                                        int& additiveXBits, int& additiveYBits, int& additiveZBits) {
    // Phase 3: replace a weaker wind modifier. Preserve additive wind while
    // deriving the new base adjustment in world coordinates from the desired vector.
    if (speed > AsPartyItemModifiers(out)->windSpeed) {
        AsPartyItemModifiers(out)->windSpeed = speed;
        AsPartyItemModifiers(out)->windHeading = self->windHeading;
        {
            wind = GetFanWindSingleton();
            additiveXBits = *(int*)&AsCWind(wind)->additiveWind.x;
            additiveYBits = *(int*)&AsCWind(wind)->additiveWind.y;
            additiveZBits = *(int*)&AsCWind(wind)->additiveWind.z;
            savedAdditiveWindBits.x = additiveXBits;
            savedAdditiveWindBits.y = additiveYBits;
            savedAdditiveWindBits.z = additiveZBits;
        }
        zeroAdditiveWind[0] = kFloat_0_PartyItem;
        zeroAdditiveWind[1] = zeroAdditiveWind[0];
        zeroAdditiveWind[2] = zeroAdditiveWind[0];
        __as__7WVectorFRC7WVector(AsWindAdjustment(GetFanWindSingleton())->additiveAdjustment, zeroAdditiveWind);
        GetWindVector__5CWindFv(unmodifiedWindBits, GetFanWindSingleton());
        __mi__FRC7WVectorRC7WVector(baseWindDeltaBits, &desiredWind, unmodifiedWindBits);
        __as__7WVectorFRC7WVector(AsWindAdjustment(GetFanWindSingleton())->baseAdjustment, baseWindDeltaBits);
        __as__7WVectorFRC7WVector(AsWindAdjustment(GetFanWindSingleton())->additiveAdjustment, &savedAdditiveWindBits);
    }
}

} // namespace ns0

// Apply the owner fan gesture to opponents as bounded wind heading/speed and refresh its effects.
int CPartyItemFujinUchiwa::UpdateOpponentWind(void* unused, void* out, int index) {
    using namespace ns0;
    // Phase 1: poll the owner's gesture only while processing an opponent.
    if (index == ownerIndex)
        return 1;
    float headingRotation[12];
    Vec3 desiredWind;
    Vec3i savedAdditiveWindBits;
    float resetBaseWind[3];
    Vec3 forwardAxis;
    float zeroAdditiveWind[3];
    int baseWindDeltaBits[3];
    int unmodifiedWindBits[3];
    int effectWindBits[3];
    int currentWindBits[3];
    float zeroEffectOffset[3];
    float steering = kFloat_0_PartyItem;
    float magnitude = kFloat_1_PartyItem;
    void* actor = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), ownerIndex & 0xFF);
    void* input = *(void**)actor;
    void* state = Object_GetSlotEntry(PartyItemSlots_GetInstance(), input);
    inputState = state;
    if (AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & kPartyItemInputStates) {
        EnsureFanInputManager();
        int pressed = Input_IsActionButtonDown(GetInputManagerSlot(), kActionUsePartyItem, input);
        if (pressed) {
            EnsureFanInputManager();
            if (Input_IsActionPressed(GetInputManagerSlot(), kActionUsePartyItem, input)) {
                PartyItem_ResetIds(state);
                resetBaseWind[0] = kFloat_0_PartyItem;
                resetBaseWind[1] = resetBaseWind[0];
                resetBaseWind[2] = resetBaseWind[0];
                __as__7WVectorFRC7WVector(AsWindAdjustment(GetFanWindSingleton())->baseAdjustment, resetBaseWind);
                AsFanGestureResult(state)->magnitude = kFloat_1_PartyItem;
                AsFanGestureResult(state)->steering = kFloat_0_PartyItem;
            } else {
                PartyItemMotion_DetectGesture(state, kFanGesture);
                magnitude = AsFanGestureResult(state)->magnitude;
                steering = AsFanGestureResult(state)->steering;
            }
        }
    }
    // Phase 2: clamp heading to aim +/- pi/2 radians and speed to the configured maximum.
    float aimHeading = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->headingRadians;
    if (magnitude < kFanGestureMinimumMagnitude)
        return 1;
    int gestureBranch = kFanNoGesture;
    if (steering < -s_fanSteeringDeadZone) {
        float lower = aimHeading - kFanMaximumHeadingDeviation;
        float t = windHeading - magnitude * kFloat_0_01_FanHeading;
        windHeading = t;
        if (t < lower)
            windHeading = lower;
        gestureBranch = kFanNegativeSteering;
    } else if (steering > s_fanSteeringDeadZone) {
        float upper = aimHeading + kFanMaximumHeadingDeviation;
        float t = windHeading + magnitude * kFloat_0_01_FanHeading;
        windHeading = t;
        if (t > upper)
            windHeading = upper;
        gestureBranch = kFanPositiveSteering;
    } else {
        gestureBranch = kFanNeutralSteering;
    }
    forwardAxis.x = kFloat_0_PartyItem;
    forwardAxis.y = forwardAxis.x;
    forwardAxis.z = kFloat_1_PartyItem;
    Matrix_SetAxisRotation(&headingRotation, windHeading, 1);
    Matrix_TransformPoint(&desiredWind, &forwardAxis, &headingRotation);
    float requestedWindSpeed = magnitude * g_fanGestureWindSpeedScale;
    float speed = kFanMaximumWindSpeed;
    // The original also keeps the requested value when the comparison is unordered.
    if (requestedWindSpeed > speed)
        goto windSpeedClamped;
    speed = requestedWindSpeed;
windSpeedClamped:
    __amu__7WVectorFf(&desiredWind, speed);
    // Phase 3: publish a stronger wind modifier while retaining additive wind.
    // Caller-owned integer copies retain the original vector-load register order.
    void* wind;
    int additiveXBits, additiveYBits, additiveZBits;
    ApplyFanWindModifier(this, out, speed, desiredWind, savedAdditiveWindBits, zeroAdditiveWind, unmodifiedWindBits,
                         baseWindDeltaBits, wind, additiveXBits, additiveYBits, additiveZBits);
    // Phase 4: propagate wind to effects and refresh the sound/sequence on cooldown.
    GetWindVector__5CWindFv(currentWindBits, GetFanWindSingleton());
    effectWindBits[0] = currentWindBits[0];
    effectWindBits[1] = currentWindBits[1];
    effectWindBits[2] = currentWindBits[2];
    ((CFx*)g_fx)->SetWindVector(effectWindBits);
    zeroEffectOffset[0] = kFloat_0_PartyItem;
    zeroEffectOffset[1] = zeroEffectOffset[0];
    zeroEffectOffset[2] = zeroEffectOffset[0];
    __as__7WVectorFRC7WVector(effectOffset, zeroEffectOffset);
    effectAttachmentMatrix = s_cameraTransform;
    effectOffsetPointer = effectOffset;
    int effectSequenceHandle = effectHandleValue;
    if (effectSequenceHandle && ((CFx*)g_fx)->HasSequence(effectSequenceHandle)) {
        if (--cooldown > 0)
            goto done;
    }
    if (gestureBranch != kFanNoGesture) {
        void* effectHandle = ((CFx*)g_fx)
                                 ->CreateAttachedEffect((Vec2_8010*)(&effectAttachment),
                                                        partyGoodsEffectStrings + kFanEffectSequence, 0, 0, 0);
        this->effectHandle = effectHandle;
        ((void (*)(void*, const char*, int, int, const char*, int, float, float, float))Sound_PlayByName)(
            g_soundManager, partyGoodsEffectStrings + kFanWindSound, 0, 0, partyGoodsEffectStrings + kFanSoundAlias, 0,
            kFloat_0_5_PartyEffects, kFloat_200_PartyEffects, kFloat_1_PartyItem);
        cooldown = kFanEffectCooldownTicks;
    }
done:
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Create a 20-point ribbon using the owner color and initial pointer position.
CPartyItemRakugakiCrayon::CPartyItemRakugakiCrayon(int index, void* resource) {
    using namespace ns0;
    PartyIItem_ctor(this, (void*)index, kRakugakiCrayon, kGestureItem);
    ribbon = 0;
    vtbl = __vt__Q25party24CPartyItemRakugakiCrayon;
    void* newRibbon = __nw__FUl(0x58);
    if (newRibbon)
        TrailRibbon_Ctor(newRibbon, resource, kRibbonPointCount, kRibbonPointCount);
    ribbon = newRibbon;
    struct Desc {
        unsigned char c[8];
        float a, b;
        int x, y, z, w;
    } style;
    float ribbonWidth = kFloat_15_PartyEffects;
    float ribbonOpacity = kFloat_0_5_PartyEffects;
    int playerColorOffset = index * 4;
    unsigned char c0 = ((unsigned char*)partyCrayonPlayerColors)[playerColorOffset];
    unsigned char* p = (unsigned char*)partyCrayonPlayerColors + playerColorOffset;
    unsigned char c1 = p[1];
    unsigned char c2 = p[2];
    unsigned char c3 = p[3];
    style.c[0] = c0;
    style.c[1] = c1;
    style.c[2] = c2;
    style.c[3] = c3;
    style.c[4] = c0;
    style.c[5] = c1;
    style.c[6] = c2;
    style.c[7] = c3;
    style.b = ribbonOpacity;
    style.w = kRibbonPointCount;
    style.x = 0;
    style.y = 1;
    style.z = 0;
    style.a = ribbonWidth;
    TrailRibbon_SetStyle(ribbon, &style);
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                          s_inputManagerDtorRegistration_partyItems);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* inputManager = GetInputManagerSlot();
    void* actor = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), ownerIndex & 0xFF);
    float out[2];
    InputManager_GetPointerPosition(inputManager, out, *(void**)actor);
    TrailRibbon_SetPointerPosition(ribbon, out);
    TrailRibbon_Update(ribbon);
}
#pragma pop

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
namespace ns0 {
// Release every ribbon buffer, stop the crayon sound and destroy the item.
void* party_CPartyItemRakugakiCrayon_dtor(void* self, int deletingFlag) {
    int i;
    int off;
    int zero;
    void* child;
    if (self) {
        child = AsCPartyItemRakugakiCrayon(self)->ribbon;
        AsCPartyItemRakugakiCrayon(self)->vtbl = __vt__Q25party24CPartyItemRakugakiCrayon;
        if (child) {
            if (AsPartyRibbonState(child)->pointPositions) {
                __dla__FPv(AsPartyRibbonState(child)->pointPositions);
                AsPartyRibbonState(child)->pointPositions = 0;
            }
            i = 0;
            off = 0;
            zero = 0;
            for (; i < AsPartyRibbonState(child)->bufferCount; off += 4, ++i) {
                if (*(void**)((char*)AsPartyRibbonState(child)->baseVertexTriplets + off)) {
                    __dla__FPv(*(void**)((char*)AsPartyRibbonState(child)->baseVertexTriplets + off));
                    *(void**)((char*)AsPartyRibbonState(child)->baseVertexTriplets + off) = 0;
                }
                if (*(void**)((char*)AsPartyRibbonState(child)->fadedVertexTriplets + off)) {
                    __dla__FPv(*(void**)((char*)AsPartyRibbonState(child)->fadedVertexTriplets + off));
                    *(void**)((char*)AsPartyRibbonState(child)->fadedVertexTriplets + off) = 0;
                }
            }
            if (AsPartyRibbonState(child)->baseVertexTriplets) {
                __dla__FPv(AsPartyRibbonState(child)->baseVertexTriplets);
                AsPartyRibbonState(child)->baseVertexTriplets = 0;
            }
            if (AsPartyRibbonState(child)->fadedVertexTriplets) {
                __dla__FPv(AsPartyRibbonState(child)->fadedVertexTriplets);
                AsPartyRibbonState(child)->fadedVertexTriplets = 0;
            }
            if (AsPartyRibbonState(child)->pointHeadings) {
                __dla__FPv(AsPartyRibbonState(child)->pointHeadings);
                AsPartyRibbonState(child)->pointHeadings = 0;
            }
            __dl__FPv(child);
            AsCPartyItemRakugakiCrayon(self)->ribbon = 0;
        }
        Sound_StopByAlias(g_soundManager, partyGoodsEffectStrings + kCrayonSoundAlias);
        party_IItem_dtor(self, 0);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
namespace ns0 {
// Crayon pre-shot hook; ribbon state is updated separately.
void party_CPartyItemRakugakiCrayon_vfunc_3() {}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Feed held-action pointer positions to the crayon ribbon and play its drawing sound.
int CPartyItemRakugakiCrayon::UpdateDrawing(void* unused, void* unused2, int index) {
    using namespace ns0;
    if (index != ownerIndex) {
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                              s_inputManagerDtorRegistration_partyItems);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        void* c8 = GetInputManagerSlot();
        void* actor = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), ownerIndex & 0xFF);
        int pressed = Input_IsActionButtonDown(c8, kActionUsePartyItem, *(void**)actor);
        if (pressed) {
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                                  s_inputManagerDtorRegistration_partyItems);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            void* c8b = GetInputManagerSlot();
            actor = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), ownerIndex & 0xFF);
            void* out;
            if (InputManager_GetPointerPosition(c8b, &out, *(void**)actor))
                TrailRibbon_SetPointerPosition(ribbon, &out);
        }
    }
    TrailRibbon_Update(ribbon);
    if (((CrayonRibbonActivity*)ribbon)->drawing) {
        if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, partyGoodsEffectStrings + kCrayonSoundAlias))
            ((void (*)(void*, const char*, int, int, const char*, int, float, float, float))Sound_PlayByName)(
                g_soundManager, partyGoodsEffectStrings + kCrayonDrawSound, 0, 0,
                partyGoodsEffectStrings + kCrayonSoundAlias, 0, kFloat_0_5_PartyEffects, kFloat_200_PartyEffects,
                kFloat_1_PartyItem);
    }
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Rebuild the crayon ribbon geometry.
int CPartyItemRakugakiCrayon::DrawRibbon() {
    using namespace ns0;
    TrailRibbon_Build(ribbon);
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize the gesture-driven tickle item with zero strength and active ticks.
CPartyItemPululuJarashi::CPartyItemPululuJarashi(void* arg) {
    using namespace ns0;
    PartyIItem_ctor(this, arg, kPululuJarashi, kGestureItem);
    float z = kFloat_0_PartyItem;
    vtbl = __vt__Q25party23CPartyItemPululuJarashi;
    strength = z;
    activeTicks = 0;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Release the party-item base; the compiler adds the delete-if-requested wrapper.
CPartyItemPululuJarashi::~CPartyItemPululuJarashi() {
    using namespace ns0;
    party_IItem_dtor(this, 0);
}
#pragma pop

#pragma push
namespace ns0 {
// Reset the tickle item active-tick counter.
void party_CPartyItemPululuJarashi_vfunc_3(void* p) {
    AsCPartyItemPululuJarashi(p)->activeTicks = 0;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes off
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Charge tickle strength from gestures, decay it each tick and apply it to the current opponent.
int CPartyItemPululuJarashi::UpdateTickleStrength(void* unused, void* destination, int index) {
    using namespace ns0;
    // Phase 1: decay the active counter; rejected owner/state/input resets it.
    char* name;
    void* state;
    int* tick = &activeTicks;
    --*tick;
    if (*tick < 0)
        *tick = 0;

    unsigned char slot = ownerIndex;
    void* actor = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), slot);
    void* input = *(void**)actor;

    // 0x8025A238: Invalid owner, game state or input shares the zero-tick path below.
    do {
        // Rejected input breaks to the reset; accepted input continues past it.
        switch (0) {
        default: {
            if (index != ownerIndex) {
                unsigned int flags = (*(unsigned int*)&AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags);
                if (((flags & kPartyItemInputStates) == 0) && ((flags & kTickleAdditionalState) == 0)) {
                    break;
                } else {
                    if (!GetInputManagerSlot()) {
                        if (!s_inputManagerInitialized) {
                            InputManager_Init(s_gameInputManager);
                            ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                                          s_inputManagerDtorRegistration_partyItems);
                            s_inputManagerInitialized = 1;
                        }
                        GetInputManagerSlot() = s_gameInputManager;
                    }
                    if (Input_IsActionButtonDown(GetInputManagerSlot(), kActionUsePartyItem, input) == 0) {
                        break;
                    } else {
                        state = Object_GetSlotEntry(PartyItemSlots_GetInstance(), input);
                        if (!GetInputManagerSlot()) {
                            if (!s_inputManagerInitialized) {
                                InputManager_Init(s_gameInputManager);
                                ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                                              s_inputManagerDtorRegistration_partyItems);
                                s_inputManagerInitialized = 1;
                            }
                            GetInputManagerSlot() = s_gameInputManager;
                        }
                        if (Input_IsActionPressed(GetInputManagerSlot(), kActionUsePartyItem, input))
                            PartyItem_ResetIds(state);
                        if (PartyItemMotion_DetectGesture(state, kTickleGesture)) {
                            name = partyGoodsEffectStrings + kTickleSound;
                            if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, name))
                                ((void (*)(void*, const char*, int, int, const char*, int, float, float,
                                           float))Sound_PlayByName)(g_soundManager, name, 0, 0, name, 0,
                                                                    kFloat_0_5_PartyEffects, kFloat_200_PartyEffects,
                                                                    kFloat_1_PartyItem);
                            float value = strength + g_tickleStrengthPerGesture;
                            value = (value > kFloat_1_PartyItem) ? kFloat_1_PartyItem : value;
                            strength = value;
                            if (!GetInputManagerSlot()) {
                                if (!s_inputManagerInitialized) {
                                    InputManager_Init(s_gameInputManager);
                                    ns0::__register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                                                  s_inputManagerDtorRegistration_partyItems);
                                    s_inputManagerInitialized = 1;
                                }
                                GetInputManagerSlot() = s_gameInputManager;
                            }
                            InputManager_StartRumble(GetInputManagerSlot(), kGestureRumbleTicks);
                            *tick += kGestureRumbleTicks;
                        }
                    }
                }
            } else {
                break;
            }
            continue;
        }
        }
        *tick = 0;
    } while (0);

    // Phase 2: publish activity, decay strength, and widen only the current
    // opponent's impact divisor. The owner's modifier is left untouched here.
    if (*tick != 0)
        AsPartyItemModifiers(destination)->tickleActive = 1;
    float value = strength - g_tickleStrengthDecayPerUpdate;
    value = (kFloat_0_PartyItem < value) ? value : kFloat_0_PartyItem;
    strength = value;
    if (index != ownerIndex && index == AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer) {
        float limit = kFloat_1_PartyItem + strength;
        float current = AsPartyItemModifiers(destination)->impactZoneWidthDivisor;
        current = (current < limit) ? limit : current;
        AsPartyItemModifiers(destination)->impactZoneWidthDivisor = current;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize the hammer item and clear its three state words.
CPartyItemStampHammer::CPartyItemStampHammer(void* arg) {
    using namespace ns0;
    PartyIItem_ctor(this, arg, kStampHammer, kGestureItem);
    unk_1C = 0;
    vtbl = __vt__Q25party21CPartyItemStampHammer;
    unk_20 = 0;
    unk_24 = 0;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns0 {
// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemStampHammer_dtor(void* self, int deletingFlag) {
    if (self != 0) {
        party_IItem_dtor(self, 0);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Lazily allocate the shared six-word hammer balloon deque and clear its selected target.
void PartyStampHammer_EnsureSharedQueue(void) {
    if (g_stampHammerBalloonQueue == 0) {
        BalloonDeque* queue = (BalloonDeque*)__nw__FUl(sizeof(BalloonDeque));
        if (queue != 0) {
            queue->base = 0;
            queue->count = 0;
            queue->blocks = 0;
            queue->start = 0;
            queue->blockCount = 0;
            queue->capacity = 0;
        }
        g_stampHammerBalloonQueue = queue;
    }
    g_stampHammerTargetBalloon = 0;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Release the owned container buffer and optionally its wrapper; entries have no element teardown here.
void* dtor_8025A638(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* p = *(void**)self;
                    if (p != 0) {
                        ((OwnedContainerBuffer*)self)->count -= (unsigned int)((OwnedContainerBuffer*)self)->count;
                        appFreeUni__FPv(p);
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Release the second container buffer through its preserved destructor expansion.
void* OwnedContainerBuffer_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            void* p = *(void**)self;
                            if (p != 0) {
                                ((OwnedContainerBuffer*)self)->count -= ((OwnedContainerBuffer*)self)->count;
                                appFreeUni__FPv(p);
                            }
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Release the shared hammer balloon deque and clear the selected target.
void CBalloonSystem::DestroySharedQueue() {
    using namespace ns1;
    void* p = g_stampHammerBalloonQueue;
    if (p != 0) {
        BalloonDeque* queue = (BalloonDeque*)p;
        int zero = 0;
        queue->count = zero;
        while ((unsigned int)queue->blockCount > 2) {
            appFreeUni__FPv(((void**)queue->blocks)[queue->start]);
            int i = queue->start + 1;
            queue->start = i;
            if ((unsigned int)i == (unsigned int)queue->capacity)
                queue->start = zero;
            queue->blockCount--;
        }
        if (queue->blockCount != 0)
            queue->base = 0x80;
    }
    p = g_stampHammerBalloonQueue;
    if (p != 0) {
        if (p != 0) {
            if (p != 0) {
                if (p != 0) {
                    if (p != 0) {
                        BlockedWordRing_dtor(p, 0);
                    }
                }
            }
            __dl__FPv(p);
        }
        g_stampHammerBalloonQueue = 0;
    }
    g_stampHammerTargetBalloon = 0;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Reset the shared hammer cooldown, target and queued balloon count while retaining deque blocks.
int party_CPartyItemStampHammer_vfunc_1(void) {
    int zero = 0;
    void* p = g_stampHammerBalloonQueue;
    s_stampHammerCooldownTicks = zero;
    if (p != 0) {
        BalloonDeque* queue = (BalloonDeque*)p;
        queue->count = zero;
        while ((unsigned int)queue->blockCount > 2) {
            appFreeUni__FPv(((void**)queue->blocks)[queue->start]);
            int i = queue->start + 1;
            queue->start = i;
            if ((unsigned int)i == (unsigned int)queue->capacity)
                queue->start = zero;
            queue->blockCount--;
        }
        if (queue->blockCount != 0)
            queue->base = 0x80;
    }
    g_stampHammerTargetBalloon = zero;
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Hammer pre-shot hook; shared queue reset occurs in its initialization hook.
void party_CPartyItemStampHammer_vfunc_3(void) {}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Transform local forward into the normalized world-space hammer direction using current aim (radians).
static inline void RefreshHammerFacingFromAim(void* self, float& aimRadians, float& reversedAimRadians,
                                              float* aimRotationMatrix, float& zeroComponent,
                                              float& forwardZComponent) {
    aimRadians = AsCGolfDoc(PartyGolfDocument())->headingRadians;
    reversedAimRadians = aimRadians >= kFloat_0_PartyItem ? aimRadians - kPi_PartyHammer : kPi_PartyHammer + aimRadians;
    Mtx44_SetRotationY(aimRotationMatrix, -reversedAimRadians);
    zeroComponent = kFloat_0_PartyItem;
    forwardZComponent = kFloat_neg1_PartyEffects;
    ((HammerVector*)((char*)self + (int)&((HammerSelf*)0)->direction))
        ->set(zeroComponent, zeroComponent, forwardZComponent);
    // Keep the float and struct views distinct: merging them changes generated address reuse.
    Mtx44_TransformDirection((float*)self + kHammerDirectionFloatIndex, (float*)self + kHammerDirectionFloatIndex,
                             aimRotationMatrix, zeroComponent);
    Vec3_Normalize(&((HammerSelf*)self)->direction, &((HammerSelf*)self)->direction);
}

// Lazily initialize/register the shared input manager, then publish its borrowed singleton pointer.
static inline void EnsureHammerInputManager() {
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_partyItems);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}

#pragma push
#pragma always_inline on
// Pop the newest borrowed pointer and trim unused tail pages, retaining the ring allocation.
static inline void PopHammerBalloonHistory(BalloonDeque*& history, void*& releaseCandidate) {
    unsigned int absolutePosition = history->base + history->count - 1;
    releaseCandidate = ((void**)history->blocks[(history->start + (absolutePosition >> kDequeBlockShift)) %
                                                history->capacity])[absolutePosition & kDequeBlockMask];
    history->count = history->count - 1;
    unsigned int usableHistoryEnd = history->blockCount == 0 ? 0 : ((history->blockCount - 1) << kDequeBlockShift);
    unsigned int unusedTailWords = usableHistoryEnd - (history->base + history->count);
    while (unusedTailWords >= kDequeTrimThreshold) {
        volatile AllocatorTag allocatorState;
        allocatorState.value = 0;
        history->blockCount = history->blockCount - 1;
        unsigned int blockRingIndex = (history->start + history->blockCount - 1) % history->capacity;
        void** blockRing = history->blocks;
        appFreeUni__FPv(blockRing[blockRingIndex]);
        blockRing[blockRingIndex] = 0;
        unusedTailWords -= kPointersPerDequeBlock;
    }
}
#pragma pop
} // namespace ns1

// Select a balloon with the prior aim, process the gesture, and release eligible history after cooldown.
int CPartyItemStampHammer::UpdateBalloons(int unused, int unused2, int index, int suppressHistoryRelease) {
    using namespace ns1;
    // Phase 1: select using the stored facing direction, THEN refresh direction
    // from current aim. This ordering intentionally uses the previous direction.
    PartyStampHammer_SelectNearestBalloonInRange(this);
    // Reused scalar: scene-object handle for reset, actor handle for gesture input.
    int ownerHandle;
    int ownerActor = *(int*)GolfPlayer_GetEntryByIndex(PartyGolfDocument(), ownerIndex);
    ownerHandle = Object_GetSlotEntry(PartyItemSlots_GetInstance(), ownerActor);
    EnsureHammerInputManager();
    if (Input_IsActionPressed(GetInputManagerSlot(), kActionUsePartyItem, ownerActor))
        PartyItem_ResetIds(ownerHandle);

    float aimRadians, reversedAimRadians;
    float aimRotationMatrix[16];
    float zeroComponent, forwardZComponent;
    RefreshHammerFacingFromAim(this, aimRadians, reversedAimRadians, aimRotationMatrix, zeroComponent,
                               forwardZComponent);
    if (--s_stampHammerCooldownTicks <= 0)
        s_stampHammerCooldownTicks = 0;

    // Phase 2: an accepted owner gesture invokes the target and pushes its pointer
    // onto the shared history, then clears the current selection.
    if (((*(unsigned int*)&AsCGolfDoc(PartyGolfDocument())->gameStateFlags) & kPartyItemInputStates) &&
        g_stampHammerTargetBalloon && index != ownerIndex) {
        int gestureAccepted;
        ownerHandle = *(int*)GolfPlayer_GetEntryByIndex(PartyGolfDocument(), ownerIndex);
        EnsureHammerInputManager();
        gestureAccepted = hammerHeld(ownerHandle);
        if (gestureAccepted) {
            void* targetBalloon = (void*)g_stampHammerTargetBalloon;
            if (targetBalloon) {
                // Slot v1 runs before enqueue; its full balloon-state contract is unresolved.
                ((VObj*)targetBalloon)->v1();
                BalloonDeque* history = (BalloonDeque*)g_stampHammerBalloonQueue;
                if (history) {
                    volatile AllocatorTag allocatorState;
                    allocatorState.value = 0;
                    unsigned int usableHistoryEnd =
                        history->blockCount == 0 ? 0 : ((history->blockCount - 1) << kDequeBlockShift);
                    if (usableHistoryEnd - (history->base + history->count) == 0)
                        BlockedWordRing_EnsureAdditionalCapacity(history, 1);
                    int blockRingCapacity;
                    unsigned int absolutePosition;
                    void** blockRingBegin = history->blocks;
                    void** firstBlockSlot;
                    void** blockRingEnd = blockRingBegin + history->capacity;
                    firstBlockSlot = blockRingBegin + history->start;
                    absolutePosition = history->count;
                    absolutePosition += history->base;
                    struct Iterator {
                        void** cur;
                        void** begin;
                        void** start;
                        void** end;
                    };
                    volatile Iterator pushIterator;
                    pushIterator.begin = blockRingBegin;
                    pushIterator.cur = firstBlockSlot;
                    pushIterator.start = firstBlockSlot;
                    pushIterator.end = blockRingEnd;
                    blockRingCapacity = blockRingEnd - blockRingBegin;
                    int blockRingIndex =
                        (firstBlockSlot - blockRingBegin) + (int)(absolutePosition >> kDequeBlockShift);
                    if (blockRingIndex < 0)
                        blockRingIndex += blockRingCapacity;
                    else if (blockRingCapacity <= blockRingIndex)
                        blockRingIndex -= blockRingCapacity;
                    void** blockSlot = pushIterator.begin + blockRingIndex;
                    pushIterator.cur = blockSlot;
                    void** blockElements = hammerBlock(history, blockSlot);
                    void** targetSlot = blockElements + (absolutePosition & kDequeBlockMask);
                    new (targetSlot) void*(*(void* volatile*)&targetBalloon);
                    history->count = history->count + 1;
                }
                s_stampHammerCooldownTicks = *(int*)&s_stampHammerReleaseDelayTicks;
            }
            g_stampHammerTargetBalloon = 0;
        }
    }
    // Phase 3: when permitted, pop history from the BACK until one eligible
    // balloon is released. Skip stale entries; trim unused pages while popping.
    if (!suppressHistoryRelease && s_stampHammerCooldownTicks <= 0 && g_stampHammerBalloonQueue) {
        while (((BalloonDeque*)g_stampHammerBalloonQueue)->count) {
            void* releaseCandidate;
            BalloonDeque* history = *(BalloonDeque* volatile*)&g_stampHammerBalloonQueue;
            volatile AllocatorTag allocatorState;
            allocatorState.value = 0;
            PopHammerBalloonHistory(history, releaseCandidate);
            if (Balloon_IsActive(releaseCandidate) && Balloon_GetCurrentScaleIndex(releaseCandidate) < 0) {
                // Slot v2 runs only for an active balloon with negative scale index.
                ((VObj*)releaseCandidate)->v2();
                break;
            }
        }
        if (((BalloonDeque*)g_stampHammerBalloonQueue)->count)
            s_stampHammerCooldownTicks = *(int*)&s_stampHammerReleaseDelayTicks;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Hammer geometry hook; no extra geometry work is required.
int party_CPartyItemStampHammer_vfunc_5(void) {
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Hammer finalization hook; no per-frame finalization is required.
void party_CPartyItemStampHammer_vfunc_6(void) {}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Map an absolute 256-word-page position into the deque pointer ring, correcting one wrap in either direction.
static inline void WrapHammerHistoryBlockSlot(int& ringScratch, void**& ringEnd, void**& ringBegin, int& ringSlotIndex,
                                              unsigned int& absolutePosition, void**& firstBlockSlot) {
    ringScratch = ringEnd - ringBegin;
    ringSlotIndex = (int)(absolutePosition >> kDequeBlockShift);
    ringSlotIndex += firstBlockSlot - ringBegin;
    if (ringSlotIndex < 0)
        ringSlotIndex += ringScratch;
    else if (ringSlotIndex >= ringScratch)
        ringSlotIndex -= ringScratch;
}

// Materialize the first history iterator using caller-owned scratch and the preserved volatile write order.
static inline void BuildHammerHistoryBegin(BalloonDeque*& history, unsigned int& absolutePosition, DequeRaw& raw,
                                           unsigned int& allocatedBlockCount, int& ringSlotIndex, void**& blockp,
                                           int& ringScratch, unsigned int*& blockElements, void**& ringBeginCopy,
                                           void**& ringStartCopy, void**& ringEndCopy, void**& ringBegin,
                                           void**& firstBlockSlot, void**& ringEnd, DequeIter& it) {
    ringBegin = history->blocks;
    ringEnd = ringBegin + history->capacity;
    ringSlotIndex = history->start;
    firstBlockSlot = ringBegin + ringSlotIndex;
    ((volatile DequeRaw*)&raw)->begin = ringBegin;
    ((volatile DequeRaw*)&raw)->cur = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->start = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->end = ringEnd;
    // ringScratch holds the ring slot COUNT until wrapping is complete.
    WrapHammerHistoryBlockSlot(ringScratch, ringEnd, ringBegin, ringSlotIndex, absolutePosition, firstBlockSlot);
    allocatedBlockCount = history->blockCount;
    // Reuse the same scalar as a ring slot BYTE ADDRESS after wrapping.
    ringScratch = (int)((volatile DequeRaw*)&raw)->begin;
    ringScratch += ringSlotIndex * sizeof(void*);
    blockp = (void**)ringScratch;
    ringBeginCopy = ((volatile DequeRaw*)&raw)->begin;
    ringStartCopy = ((volatile DequeRaw*)&raw)->start;
    ((volatile DequeRaw*)&raw)->cur = blockp;
    ringEndCopy = ((volatile DequeRaw*)&raw)->end;
    blockElements = allocatedBlockCount == 0 ? 0 : (unsigned int*)*blockp;
    it.cur = blockp;
    it.begin = ringBeginCopy;
    it.start = ringStartCopy;
    it.end = ringEndCopy;
    it.elem = blockElements + (absolutePosition & kDequeBlockMask);
}

// Materialize the end history position in caller-owned scratch; no queue ownership is transferred.
static inline void BuildHammerHistoryEnd(BalloonDeque*& history, unsigned int& absolutePosition, DequeRaw& raw,
                                         unsigned int& allocatedBlockCount, int& ringSlotIndex, void**& blockp,
                                         int& ringScratch, unsigned int*& blockElements, void**& ringBeginCopy,
                                         void**& ringStartCopy, void**& ringEndCopy, void**& ringBegin,
                                         void**& firstBlockSlot, void**& ringEnd, DequeIter& last) {
    ringBegin = history->blocks;
    ringEnd = ringBegin + history->capacity;
    ringSlotIndex = history->start;
    firstBlockSlot = ringBegin + ringSlotIndex;
    ((volatile DequeRaw*)&raw)->cur = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->begin = ringBegin;
    ((volatile DequeRaw*)&raw)->start = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->end = ringEnd;
    // ringScratch holds the ring slot COUNT until wrapping is complete.
    WrapHammerHistoryBlockSlot(ringScratch, ringEnd, ringBegin, ringSlotIndex, absolutePosition, firstBlockSlot);
    allocatedBlockCount = history->blockCount;
    // Reuse the same scalar as a ring slot BYTE ADDRESS after wrapping.
    ringScratch = (int)((volatile DequeRaw*)&raw)->begin;
    ringScratch += ringSlotIndex * sizeof(void*);
    blockp = (void**)ringScratch;
    ringBeginCopy = ((volatile DequeRaw*)&raw)->begin;
    ringStartCopy = ((volatile DequeRaw*)&raw)->start;
    ((volatile DequeRaw*)&raw)->cur = blockp;
    ringEndCopy = ((volatile DequeRaw*)&raw)->end;
    blockElements = allocatedBlockCount == 0 ? 0 : (unsigned int*)*blockp;
    ((volatile DequeIter*)&last)->cur = blockp;
    ((volatile DequeIter*)&last)->begin = ringBeginCopy;
    ((volatile DequeIter*)&last)->start = ringStartCopy;
    ((volatile DequeIter*)&last)->end = ringEndCopy;
    ((volatile DequeIter*)&last)->elem = blockElements + (absolutePosition & kDequeBlockMask);
}

// Resolve the history end element in caller-owned scratch; no queue ownership is transferred.
static inline void BuildHammerHistoryEndElement(BalloonDeque*& history, unsigned int& absolutePosition, DequeRaw& raw,
                                                unsigned int& allocatedBlockCount, int& ringSlotIndex, void**& blockp,
                                                int& ringScratch, unsigned int*& blockElements, void**& ringBegin,
                                                void**& firstBlockSlot, void**& ringEnd, unsigned int*& endElem) {
    ringBegin = history->blocks;
    ringEnd = ringBegin + history->capacity;
    ringSlotIndex = history->start;
    firstBlockSlot = ringBegin + ringSlotIndex;
    ((volatile DequeRaw*)&raw)->begin = ringBegin;
    ((volatile DequeRaw*)&raw)->cur = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->start = firstBlockSlot;
    ((volatile DequeRaw*)&raw)->end = ringEnd;
    // ringScratch holds the ring slot COUNT until wrapping is complete.
    WrapHammerHistoryBlockSlot(ringScratch, ringEnd, ringBegin, ringSlotIndex, absolutePosition, firstBlockSlot);
    allocatedBlockCount = history->blockCount;
    // Reuse the same scalar as a ring slot BYTE ADDRESS after wrapping.
    ringScratch = (int)((volatile DequeRaw*)&raw)->begin;
    ringScratch += ringSlotIndex * sizeof(void*);
    blockp = (void**)ringScratch;
    ((volatile DequeRaw*)&raw)->cur = blockp;
    blockElements = allocatedBlockCount == 0 ? 0 : (unsigned int*)*blockp;
    endElem = blockElements + (absolutePosition & kDequeBlockMask);
}

// Advance a borrowed history iterator until the requested balloon or the end, wrapping between deque pages.
static inline void FindHammerHistoryBalloon(DequeIter*& historyCursor, unsigned int*& endElement,
                                            const unsigned int*& requestedBalloon) {
    while (historyCursor->elem != endElement && *historyCursor->elem != *requestedBalloon) {
        historyCursor->elem = historyCursor->elem + 1;
        if (historyCursor->elem == (unsigned int*)((char*)*historyCursor->cur + kDequeBlockBytes)) {
            historyCursor->cur = historyCursor->cur + 1;
            if (historyCursor->cur == historyCursor->end)
                historyCursor->cur = historyCursor->begin;
            historyCursor->elem = (unsigned int*)*historyCursor->cur;
        }
    }
}

// Select an unqueued balloon by horizontal cone, then full 3D distance.
// Required self supplies nonzero world facing direction; ball/balloon positions
// and borrowed list nodes must stay valid during the two scans. List is intrusive:
// sentinel address = list+4, first=list+8, node.next at +4, balloon pointer at +8.
// Empty/absent list or absent history returns without changing the old target.
// Only g_stampHammerTargetBalloon is published; no enqueue, balloon call or RNG.
// Queue membership compares pointer bits, not balloon IDs. Expanded iterator
// blocks below ONLY implement that predicate; geometry starts at !alreadyQueued.
// Per pass: membership -> project positions to world XZ -> normalize -> acos
// facing angle -> wrap at +/-pi -> symmetric cone -> full XYZ distance.
// No dot clamp/zero-length fallback is supplied; coincident XZ positions can
// produce NaN and fail the cone comparisons. Do not replace with an angle shortcut.
// Equal distances keep the earlier list node. No accepted candidate keeps the
// old target, including a failed seed and refinement. Selection uses prior aim:
// the caller refreshes facing only AFTER selection (UpdateBalloons above).
void PartyStampHammer_SelectNearestBalloonInRange(void* self) {
    void* candidateNode;
    void* listSentinel;
    void* unhitList;
    unhitList = Balloon_GetUnhitBalloonQueue(self);
    if (unhitList == 0 || AsBalloonList(unhitList)->count == 0 || g_stampHammerBalloonQueue == 0)
        return;

    listSentinel = (void*)((unsigned int)unhitList + (int)&((BalloonListView*)0)->sentinel);
    candidateNode = AsBalloonList(unhitList)->first;
    float negativePi, positivePi, twoPi, minusOne;
    float refineDot, refineDirectionLength;
    float remainingNegativePi, remainingPositivePi, remainingTwoPi, remainingMinusOne;
    negativePi = kNegativePi_PartyHammer;
    twoPi = kTwoPi_PartyHammer;
    minusOne = kFloat_neg1_PartyEffects;
    positivePi = kPi_PartyHammer;
    // Phase 1: seed with the first unqueued balloon inside the horizontal cone.
    // The bound starts in radians and becomes full 3D world distance on success.
    // If no seed is found, the second pass keeps the numeric radian value as its
    // distance cutoff. Preserve this existing fallback rather than inventing infinity.
    float coneRadiansThenNearestDistance = kHammerTargetHalfConeRadians;
    int seedZero = 0;
    while (candidateNode != listSentinel) {
        bool alreadyQueued;
        {
            // Exclusion test: materialize history begin/listSentinel iterators, scan
            // borrowed balloon pointers, and rebuild listSentinel for the final comparison.
            // Each iterator carries a ring-slot pointer and an in-page element.
            const unsigned int* value = &AsBalloonListNode(candidateNode)->balloon;
            DequeIter it;
            DequeIter last;
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->base;
                DequeRaw raw;
                BuildHammerHistoryBegin(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                        ringScratch, blockElements, ringBeginCopy, ringStartCopy, ringEndCopy,
                                        ringBegin, firstBlockSlot, ringEnd, it);
            }
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->count;
                absolutePosition += history->base;
                DequeRaw raw;
                BuildHammerHistoryEnd(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                      ringScratch, blockElements, ringBeginCopy, ringStartCopy, ringEndCopy, ringBegin,
                                      firstBlockSlot, ringEnd, last);
            }
            DequeIter* v = &it;
            unsigned int* stop = last.elem;
            FindHammerHistoryBalloon(v, stop, value);
            unsigned int* found = ((volatile DequeIter*)&it)->elem;
            unsigned int* endElem;
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->count;
                absolutePosition += history->base;
                DequeRaw raw;
                BuildHammerHistoryEndElement(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                             ringScratch, blockElements, ringBegin, firstBlockSlot, ringEnd, endElem);
            }
            alreadyQueued = found != endElem;
        }
        if (!alreadyQueued) {
            void* candidateBalloon = (void*)AsBalloonListNode(candidateNode)->balloon;
            float ballOnGroundPlane[3];
            float horizontalDirectionToBalloon[3] = {0.0f, 0.0f, 0.0f};
            horizontalDirectionToBalloon[0] = ((float*)Balloon_GetPosition(candidateBalloon))[0];
            horizontalDirectionToBalloon[2] = ((float*)Balloon_GetPosition(candidateBalloon))[2];
            *(unsigned int*)&ballOnGroundPlane[0] = 0;
            *(unsigned int*)&ballOnGroundPlane[1] = 0;
            *(unsigned int*)&ballOnGroundPlane[2] = 0;
            ballOnGroundPlane[0] = ((float*)GolfBall__Fv())[0];
            ballOnGroundPlane[2] = ((float*)GolfBall__Fv())[2];
            Vec3_Subtract(horizontalDirectionToBalloon, horizontalDirectionToBalloon, ballOnGroundPlane);
            Vec3_Normalize(horizontalDirectionToBalloon, horizontalDirectionToBalloon);
            float dot = Vec3_Dot(((HammerSelf*)self)->direction, horizontalDirectionToBalloon);
            float directionLength = Vec3_Length(horizontalDirectionToBalloon);
            float facingLength = Vec3_Length(((HammerSelf*)self)->direction);
            float angle = (float)acos(dot / (facingLength * directionLength));
            if (angle <= negativePi)
                angle = twoPi + angle;
            else if (angle >= positivePi)
                angle = minusOne * (twoPi - angle);
            int insideFacingCone = seedZero;
            if (angle <= coneRadiansThenNearestDistance && angle >= -coneRadiansThenNearestDistance)
                insideFacingCone = 1;
            if (insideFacingCone) {
                g_stampHammerTargetBalloon = (int)AsBalloonListNode(candidateNode)->balloon;
                void* ballWorldPosition = GolfBall__Fv();
                void* balloonWorldPosition = Balloon_GetPosition((void*)g_stampHammerTargetBalloon);
                float ballToBalloon[3];
                __mi__FRC7WVectorRC7WVector(ballToBalloon, balloonWorldPosition, ballWorldPosition);
                coneRadiansThenNearestDistance = Magnitude__7WVectorCFv(ballToBalloon);
                break;
            }
        }
        candidateNode = AsBalloonListNode(candidateNode)->next;
    }

    // Phase 2: refine by full 3D world distance; ties keep the earlier balloon.
    // After a failed seed scan, sentinel->next restarts at the first candidate.
    candidateNode = AsBalloonListNode(candidateNode)->next;
    listSentinel = (char*)unhitList + (int)&((BalloonListView*)0)->sentinel;
    remainingNegativePi = kNegativePi_PartyHammer;
    int refineZero = 0;
    remainingTwoPi = kTwoPi_PartyHammer;
    remainingMinusOne = kFloat_neg1_PartyEffects;
    remainingPositivePi = kPi_PartyHammer;
    float remainingHalfConeRadians = kHammerTargetHalfConeRadians;
    // Entry and back edge share ONE sentinel check below. A conventional while
    // introduces extra operations under these compiler options; retain this
    // two-label form so the accepted instruction/data-flow proof stays intact.
    // The body advances exactly one candidate on every iteration.
    goto checkRemainingBalloon;
scanRemainingBalloon:
    {
        bool alreadyQueued;
        {
            const unsigned int* value = &AsBalloonListNode(candidateNode)->balloon;
            DequeIter it;
            DequeIter last;
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->base;
                DequeRaw raw;
                BuildHammerHistoryBegin(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                        ringScratch, blockElements, ringBeginCopy, ringStartCopy, ringEndCopy,
                                        ringBegin, firstBlockSlot, ringEnd, it);
            }
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->count;
                absolutePosition += history->base;
                DequeRaw raw;
                BuildHammerHistoryEnd(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                      ringScratch, blockElements, ringBeginCopy, ringStartCopy, ringEndCopy, ringBegin,
                                      firstBlockSlot, ringEnd, last);
            }
            DequeIter* v = &it;
            unsigned int* stop = last.elem;
            FindHammerHistoryBalloon(v, stop, value);
            unsigned int* found = ((volatile DequeIter*)&it)->elem;
            unsigned int* endElem;
            {
                unsigned int allocatedBlockCount;
                int ringSlotIndex;
                void** blockp;
                int ringScratch;
                unsigned int absolutePosition;
                unsigned int* blockElements;
                void** ringBeginCopy;
                void** ringStartCopy;
                void** ringEndCopy;
                void** ringBegin;
                void** firstBlockSlot;
                void** ringEnd;
                BalloonDeque* history;
                history = (BalloonDeque*)g_stampHammerBalloonQueue;
                absolutePosition = history->count;
                absolutePosition += history->base;
                DequeRaw raw;
                BuildHammerHistoryEndElement(history, absolutePosition, raw, allocatedBlockCount, ringSlotIndex, blockp,
                                             ringScratch, blockElements, ringBegin, firstBlockSlot, ringEnd, endElem);
            }
            alreadyQueued = found != endElem;
        }
        if (!alreadyQueued) {
            void* candidateBalloon = (void*)AsBalloonListNode(candidateNode)->balloon;
            float ballOnGroundPlane[3];
            float horizontalDirectionToBalloon[3] = {0.0f, 0.0f, 0.0f};
            horizontalDirectionToBalloon[0] = ((float*)Balloon_GetPosition(candidateBalloon))[0];
            horizontalDirectionToBalloon[2] = ((float*)Balloon_GetPosition(candidateBalloon))[2];
            *(unsigned int*)&ballOnGroundPlane[0] = 0;
            *(unsigned int*)&ballOnGroundPlane[1] = 0;
            *(unsigned int*)&ballOnGroundPlane[2] = 0;
            ballOnGroundPlane[0] = ((float*)GolfBall__Fv())[0];
            ballOnGroundPlane[2] = ((float*)GolfBall__Fv())[2];
            Vec3_Subtract(horizontalDirectionToBalloon, horizontalDirectionToBalloon, ballOnGroundPlane);
            Vec3_Normalize(horizontalDirectionToBalloon, horizontalDirectionToBalloon);
            refineDot = Vec3_Dot(((HammerSelf*)self)->direction, horizontalDirectionToBalloon);
            refineDirectionLength = Vec3_Length(horizontalDirectionToBalloon);
            float facingLength = Vec3_Length(((HammerSelf*)self)->direction);
            float angle = (float)acos(refineDot / (facingLength * refineDirectionLength));
            if (angle <= remainingNegativePi)
                angle = remainingTwoPi + angle;
            else if (angle >= remainingPositivePi)
                angle = remainingMinusOne * (remainingTwoPi - angle);
            int insideFacingCone = 0;
            if (angle <= remainingHalfConeRadians && angle >= -remainingHalfConeRadians)
                insideFacingCone = 1;
            if (insideFacingCone) {
                void* ballWorldPosition = GolfBall__Fv();
                void* balloonWorldPosition = Balloon_GetPosition((void*)AsBalloonListNode(candidateNode)->balloon);
                float ballToBalloon[3];
                __mi__FRC7WVectorRC7WVector(ballToBalloon, balloonWorldPosition, ballWorldPosition);
                float candidateDistance = Magnitude__7WVectorCFv(ballToBalloon);
                if (candidateDistance < coneRadiansThenNearestDistance) {
                    coneRadiansThenNearestDistance = candidateDistance;
                    g_stampHammerTargetBalloon = (int)AsBalloonListNode(candidateNode)->balloon;
                }
            }
        }
        candidateNode = AsBalloonListNode(candidateNode)->next;
    }
checkRemainingBalloon:
    if (candidateNode != listSentinel)
        goto scanRemainingBalloon;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Return the container member at offset eight; its concrete owner remains unresolved.
void* Container_GetMember8(void* self) {
    return (char*)self + 8;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize the Hurricane wind range to 15-30 and seed the random generator.
CPartyItemHurricaneBadge::CPartyItemHurricaneBadge(int size) {
    using namespace ns1;
    PartyIItem_ctor(this, size, kHurricaneBadge, kBadgeItem);
    float f0 = kFloat_15_PartyEffects;
    void* vt = __vt__Q25party24CPartyItemHurricaneBadge;
    int one = 1;

    vtbl = vt;
    unk_10 = 0;
    minimum = f0;
    maximum = (kFloat_30_PartyBadge);
    windSpeedOverrideEnabled = one;
    oneMeterWindEnabled = one;
    srand(time(0));
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Destroy the party-item base.
CPartyItemHurricaneBadge::~CPartyItemHurricaneBadge() {
    using namespace ns1;
    party_IItem_dtor(this, 0);
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Choose a shared random wind modifier once; apply owner/opponent badge fields.
void CPartyItemHurricaneBadge::ApplyWindOverrides(void* out, int index) {
    using namespace ns1;
    if (kFloat_neg1_PartyEffects == s_hurricaneBadgeWindValue) {
        int n = rand();
        float fa = minimum;
        float fb = maximum;
        int frac = (int)(kFloat_1_PartyItem + (fb - fa));
        n %= frac;
        s_hurricaneBadgeWindValue = (float)((int)fa + n);
    }
    if (index == ownerIndex)
        AsPartyItemModifiers(out)->oneMeterWindEnabled = oneMeterWindEnabled;
    else {
        AsPartyItemModifiers(out)->windSpeedOverrideEnabled = windSpeedOverrideEnabled;
        AsPartyItemModifiers(out)->windSpeedOverride = s_hurricaneBadgeWindValue;
    }
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Hurricane update hook; its modifier is assigned by the pre-shot hook.
int party_CPartyItemHurricaneBadge_vfunc_4(void) {
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize owner spin limits and a shared additive launch-velocity bonus of 1.
// CGolfRule::StartBallFlightFromSample (golf_rule_round_flow_and_shot_sim.cpp) applies vy *= (1 + bonus):
// the default bonus 1 doubles launch vy. It is dimensionless, not itself a multiplier.
CPartyItemFeatherBadge::CPartyItemFeatherBadge(int size) {
    using namespace ns1;
    PartyIItem_ctor(this, size, kFeatherBadge, kBadgeItem);

    float f1 = kFloat_1_PartyItem;
    spinRangeOverrideEnabled = 1;
    vtbl = __vt__Q25party22CPartyItemFeatherBadge;
    float f0 = kFloat_30_PartyBadge;
    spinRangeY = f0;
    spinRangeX = f0;
    launchVerticalVelocityBonus = f1;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemFeatherBadge_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        party_IItem_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Copy owner spin overrides; publish the additive launch-velocity bonus for every player.
// out is a required borrowed modifier block; index is the queried player, not the
// owner. Non-owner spin fields are untouched; the launch bonus is always written.
void CPartyItemFeatherBadge::ApplyModifiersToPlayer(void* out, int index) {
    if (index == ownerIndex) {
        AsPartyItemModifiers(out)->spinRangeOverrideEnabled = spinRangeOverrideEnabled;
        AsPartyItemModifiers(out)->spinRangeY = spinRangeY;
        AsPartyItemModifiers(out)->spinRangeX = spinRangeX;
    }
    AsPartyItemModifiers(out)->launchVerticalVelocityBonus = launchVerticalVelocityBonus;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// While active, copy owner spin limits and every player's additive launch-velocity bonus.
// Unlike the pre-shot hook above, this does not set spinRangeOverrideEnabled.
// Inactive leaves out untouched. The unused integer is an ABI argument; return 1
// reports successful dispatch even when no modifier changes.
int CPartyItemFeatherBadge::ApplyModifiersIfActive(int, void* out, int index) {
    if (active != 0) {
        if (index == ownerIndex) {
            AsPartyItemModifiers(out)->spinRangeY = spinRangeY;
            AsPartyItemModifiers(out)->spinRangeX = spinRangeX;
        }
        AsPartyItemModifiers(out)->launchVerticalVelocityBonus = launchVerticalVelocityBonus;
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize a timebomb badge with elapsed zero and period 30.
CPartyItemTimebombBadge::CPartyItemTimebombBadge(int size) {
    using namespace ns1;
    PartyIItem_ctor(this, size, kTimebombBadge, kBadgeItem);
    vtbl = __vt__Q25party23CPartyItemTimebombBadge;
    elapsedTicks = 0;
    period = kDefaultTimebombPeriod;
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Destroy the party-item base and optionally free the item allocation.
void* party_CPartyItemTimebombBadge_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        party_IItem_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Reset the elapsed timebomb ticks before a shot.
void party_CPartyItemTimebombBadge_vfunc_3(void* self) {
    AsCPartyItemTimebombBadge(self)->elapsedTicks = 0;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Admit one clock tick; camera mode overrides shot-state/phase only before triggering.
// Admission table:
// triggered=0, camera!=0: advance regardless of golf state, phase or control mode.
// triggered=0, camera=0: require Aim/MotionSwing AND (button: phase==0; motion: phase<2).
// triggered=1: ignore state/camera/controls; advance while elapsed<=period*18.
// Every other row pauses. A triggered clock at/below the threshold can advance
// through threshold+1; once above the threshold it stops.
static inline void EvaluateTimebombAdmission(void* self, void* output, void*& game, void*& root, int& allowed) {
    allowed = 0;
    if (AsPartyItemModifiers(output)->timebombTriggered == 0) {
        game = *(void**)&AsCSharedDoc(root = g_sharedDoc)->golfDoc;
        if (((*(unsigned int*)&AsCGolfDoc(game)->gameStateFlags)) & kTimebombCountStates) {
            SharedPlayerControlRecord* slot = &((SharedPlayerControls*)root)->players[AsCGolfDoc(game)->currentPlayer];
            if (slot->buttonSwingSelected != kPartyMotionControls) {
                if (AsCGolfDoc(game)->shotPhase == kTimebombPhase0)
                    allowed = 1;
            } else if (AsCGolfDoc(game)->shotPhase < kTimebombPhase2)
                allowed = 1;
        }
        if (AsCGolfDoc(game)->cameraMode != 0)
            allowed = 1;
    } else if (AsCPartyItemTimebombBadge(self)->elapsedTicks <=
               AsCPartyItemTimebombBadge(self)->period * kTimebombTicksPerPeriod)
        allowed = 1;
}

// Advance the admitted clock, publish period/ticks, and trigger unless the player wears this badge.
int party_CPartyItemTimebombBadge_UpdateTimebomb(void* self, int, void* output) {
    void* game;
    void* root;
    int allowed;
    EvaluateTimebombAdmission(self, output, game, root, allowed);
    if (allowed)
        ++AsCPartyItemTimebombBadge(self)->elapsedTicks;
    AsPartyItemModifiers(output)->timebombTicks = AsCPartyItemTimebombBadge(self)->elapsedTicks;
    AsPartyItemModifiers(output)->timebombPeriod = AsCPartyItemTimebombBadge(self)->period;
    if (AsPartyItemModifiers(output)->timebombTriggered == 0 &&
        AsCPartyItemTimebombBadge(self)->elapsedTicks >=
            AsCPartyItemTimebombBadge(self)->period * kTimebombTicksPerPeriod) {
        int itemSlots = PartyItemSlots_GetInstance();
        void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        int currentPlayerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
        if (PartyItemSlots_GetItemType(itemSlots, currentPlayerIndex) != kTimebombBadge)
            AsPartyItemModifiers(output)->timebombTriggered = 1;
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Initialize item 11's one-shot random-club latch and seed the shared C RNG.
// The latch is cleared again before each shot (PartyItem_ClearActivationWord); the
// club itself is chosen by PartyGoods_ApplyWeightedRandomClub (blocked_word_ring_buffer.cpp).
// srand(time(0)) reseeds the process RNG, also used by Hurricane; keep it.
CPartyItemRandomClub::CPartyItemRandomClub(int size) {
    using namespace ns1;
    PartyIItem_ctor(this, size, kPartyItem11, kBadgeItem);
    randomClubApplied = 0;
    vtbl = s_partyItem11Vtable;
    srand(time(0));
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Destroy the party-item base.
CPartyItemRandomClub::~CPartyItemRandomClub() {
    using namespace ns1;
    party_IItem_dtor(this, 0);
}
#pragma pop

#pragma push
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
namespace ns1 {
// Clear the shared +10 badge word: Feather active flag OR item-11 applied latch.
// Both vtables bind this same store; receiver type is a compatible prefix view,
// not proof that random-club item 11 is a Feather Badge. Modifier-block clearing
// is the manager's separate responsibility (party_goods_item_display.cpp).
void PartyItem_ClearActivationWord(void* self) {
    AsCPartyItemFeatherBadge(self)->active = 0;
}
}
#pragma pop
}
