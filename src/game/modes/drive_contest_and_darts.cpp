// Driving Contest and darts scoring: the CDriveSystem actor and its result dialog, and the
// darts target and plate drawing and the player rankings for both games.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Driving Contest and darts scoring/target drawing. Mini-game actor lifecycle
// owns aim wrappers and result dialogs; golf document/ball/player data are borrowed.
// Modding lookup:
// - Driving Contest distance/bonus: CDriveSystem::HandleMsg request 0; horizontal
//   current-minus-initial trajectory distance / 3.200000047683716, truncated to
//   hundredths, plus separate 100.0 hole-in output. Scalar data: the data section.
//   Output is the distance-derived contest SCORE, in the units accumulated by
//   CGolfRule::ProcessBallFlight and compared by Drive ranking; see entry contract.
// - Single-player placing: partyResultTier from PartyGame_EvaluatePlayerThresholdTier
//   (minigame_aim_and_party_tiers.cpp), configured by TourPartyConfig_ApplyToSharedDoc. Tier is not rank.
// - Both ranking requests require 1..4 live players; scratch arrays have four
//   elements and no clamp. Grow all ranking/player-record storage together.
// - Darts: CDartsSystem::HandleMsg and dartsDistanceScoreTiers (four score/radius
//   records), sector heading/radians, hole-in 50, outside-ring -10.
// - Target texture/draw: CDartsTarget::Draw and CDartsPlates::Draw use existing
//   texture handles, stride-24 XYZ/RGBA/UV vertices and triangle strips.
// Booster gates are a research dependency, not a scoring multiplier in this unit:
// distance reads the resulting ball trajectory. Do not invent gate bonuses here.
// No resolution, viewport/scissor, aspect or presentation/frame-rate switch.
// Call HandleMsg synchronously with valid output storage; unhandled requests
// leave outputs untouched. Actor update delta is forwarded to dialog animation.
#include "game/golfball.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CGolfDoc.h"
// Borrow the golf document view; its current hole selects the aim/rank entry.
static inline CGolfDoc* AsCGolfDoc(void* object) {
    return (CGolfDoc*)object;
}
#include "classes/CSharedDoc.h"
// Borrow the shared document that owns the current golf-document pointer.
static inline CSharedDoc* AsCSharedDoc(void* object) {
    return (CSharedDoc*)object;
}

#include "sdk_protos.h"
#include "game/wvector.h"
#include "classes/CDriveSystem.h"
#include "classes/CDartsPlates.h"
#include "classes/CDartsSystem.h"
#include "classes/CDartsTarget.h"
#include <decomp/cw_bitfields.h>
#include "classes/CMiniGameAim.h"
#include "classes/CDartsDialog.h"

// Borrow the darts target render-resource view without retaining its handles.
static inline CDartsTarget* AsCDartsTarget(void* object) {
    return (CDartsTarget*)object;
}

// Borrow the darts actor's aim/scoring/dialog fields in their original layout.
static inline CDartsSystem* AsCDartsSystem(void* object) {
    return (CDartsSystem*)object;
}

// Borrow the darts plate collection and its render resources.
static inline CDartsPlates* AsCDartsPlates(void* object) {
    return (CDartsPlates*)object;
}

// Borrow the Driving Contest actor's lifecycle fields; no ownership transfer.
static inline CDriveSystem* AsCDriveSystem(void* object) {
    return (CDriveSystem*)object;
}

extern "C" {

enum DartsLimits {
    kHoleCount = 18,
    kDistanceTierCount = 4,
    kHoleInScore = 50,
    kMissScore = -10
};
enum DartsRequests {
    kDartsGetHoleTarget,
    kDartsScoreCurrentShot,
    kDartsUpdateRanks,
    kDartsGetAimPosition,
    kDartsCheckScoringRadius,
    kDartsEnsureResultDialog
};
enum DriveRequests {
    kDriveGetDistance,
    kDriveUpdateRanks,
    kDriveEnsureResultDialog
};
enum DartsDrawConstants {
    kDartsVertexFormat = 0x11F001,
    kDartsVertexStride = 0x18,
    kDartsTriangleStrip = 0x98
};
// Engine state IDs verified in gx_renderer_state_and_efb_copy.cpp CachedRenderStateId and gx_render_state_and_texture_upload.cpp
// Render_ApplyFogState/Render_ApplyDepthState. Target explicitly disables depth
// writes and uses GX LEQUAL; plates temporarily disable fog and restore it.
enum DartsStateSettings {
    kPlateFogEnabledState = 1,
    kDartsDepthCompareState = 3,
    kDartsDepthWriteState = 5,
    kDartsCompareLessEqual = 3,
    kDartsDisableDepthWrite = 0,
    kTargetValueCount = 3
};
extern unsigned char s_dartsSystemRttiDescriptor[1];
extern const float kFloat_6_2831855;
extern const float kFloat_1_5707964_tweenDartsAudio;
extern float s_dartsTwoPi;
extern const float kFloat_0_NumericMenuInput;
extern const float kDriveWorldUnitsPerScoreUnit;
extern const float kFloat_100_NumericMenuInput;
extern const float kFloat_0_darts;
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void __dl__FPv(void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern unsigned char __vt__12CDriveSystem[];
extern const unsigned int kDartsTexturedRenderStateDefaults[0xD0 / 4];
extern unsigned char kIdentityMatrix4x4[0x40];
extern unsigned char s_texturedPrimitiveStateApplyVtable[];
extern void SetTransform(int, void*);
extern void Engine_SetRenderStateList(void*, int = 15);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetType7Resource(int);
extern void dtor_800EF304(void*, int);
extern void Engine_SetCachedRenderState(int, int);
extern void Engine_GetRenderState(int, void*);
extern void Vec3_CopyToOutput(void*, void*);
extern unsigned int ISharedResource_GetLocked(void*);
extern unsigned int ISharedResource_GetPaletteObject(void*);
extern void Engine_SetTextureStageResource(int, unsigned int);
extern void Engine_SetTexturePalette(int, unsigned int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetVertexStream(int, unsigned int, int, int);
extern void DrawPrimitiveFromBoundBuffers(int, int, unsigned int);
extern void* GolfBall__Fv();
extern void* PreData__9CGolfBallFi(void*, int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void __ct__12CDartsDialogFPvPv(void*, void*, int);
extern void* GolfPlayer_GetEntryByIndex(void*, int);
extern void* __nw__FUl(unsigned int);
extern double atan2(double, double);
extern void Vec3_Subtract(void*, void*, void*);
extern float Vec3_Length(void*);
extern void Vec3_CopySourceToDestination(void*, void*);
extern void Vec3_Subtract(void*, void*, void*);
extern CSharedDoc* g_sharedDoc;
extern const unsigned char dartsDistanceScoreTiers[];
extern // Requires 1..4 players and valid per-hole aim/score data through currentHoleIndex.
    // A hole win requires unique smallest NONNEGATIVE (target-score); over-target scores
    // are excluded. Tied closest players receive no outright win. Compare more wins,
    // then smaller abs(sum(target)-sum(score)); only high nibble uses tie-break order.
    // Single player uses partyResultTier: 2/3 -> rank 1, 1 -> rank 2, otherwise rank 3.

    struct Obj10 {
    char data[16];
    Obj10();
};

// Construct the owned mini-game aim-table wrapper in its embedded storage.
Obj10::Obj10() {
    ((CMiniGameAim*)this)->__ct();
}

struct DriveVirtual {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void cleanup();
};

// Run drive-system cleanup, destroy its base, and optionally delete the object.
CDriveSystem::~CDriveSystem() {
    void* vt = (void*)__vt__12CDriveSystem;
    *(void**)this = vt;
    ((DriveVirtual*)this)->cleanup();
    __dt__18NativeBase80201D18Fv(this, 0);
}

// Return the darts-system runtime type descriptor.
void* CDartsSystem::GetRTTI() {
    return s_dartsSystemRttiDescriptor;
}

// Initialize this unit's writable two-pi constant.
void Darts_InitializeAngleConstant() {
    s_dartsTwoPi = kFloat_6_2831855;
}

// Initialize the drive contest system with no aim resource or dialog.
CDriveSystem::CDriveSystem() {
    __ct__18NativeBase80201D18Fv(this);
    *(void**)this = __vt__12CDriveSystem;
    unk_8 = 5;
    (*(int*)&miniGameAim) = 0;
    (*(int*)&dialog) = 0;
    dialogRequested = 0;
}

// Wait until the drive contest aim table finishes loading.
// Requires an aim wrapper from OnPreLoadInit. Busy-polls until complete with
// no timeout/error return; do not substitute a frame delay in this callback.
void CDriveSystem::OnLoad() {
    do {
    } while (!((CMiniGameAim*)miniGameAim)->PollLoad(0));
}

// Resolve aim entries and allow the first result dialog request.
void CDriveSystem::OnInit() {
    ((CMiniGameAim*)miniGameAim)->ResolveEntries();
    dialogRequested = 0;
}

struct Base4 {
    char pad[4];
};
struct VObj4 : Base4 {
    virtual void call(int);
};

// Replace any previous aim resource with a fresh mini-game aim table.
void CDriveSystem::OnPreLoadInit() {
    if (miniGameAim != 0) {
        if (miniGameAim != 0) {
            ((VObj4*)miniGameAim)->call(1);
        }
        miniGameAim = 0;
    }
    miniGameAim = new Obj10();
}

// Update the result dialog and delete it when it finishes.
void CDriveSystem::OnProcess(float delta) {
    void* ptr = dialog;
    if (ptr != 0 && ((CDartsDialog*)ptr)->Update(delta) != 0) {
        ptr = dialog;
        if (ptr != 0) {
            struct DriveDelete {
                virtual ~DriveDelete() DECOMP_LOCAL_VIRTUAL;
            };
            delete (DriveDelete*)ptr;
            dialog = 0;
        }
    }
}

// Draw the drive contest result dialog when present.
void CDriveSystem::OnDisplay() {
    void* ptr = dialog;
    if (ptr != 0) {
        ((CDartsDialog*)ptr)->Draw();
    }
}

struct Obj40 {
    char data[0x40];
    Obj40(void*);
};

struct VObj0 {
    virtual void call(int);
};

// Construct a drive result dialog from the first nonzero entry in reversed order.
// resource borrows the aim wrapper; all three words encode target values, not
// owned pointers (CDartsDialog::CDartsDialog, darts_dialog_and_load_data_menu.cpp). clearType=1 selects drive mode.
// Target-value contract (also DartsValues below): signed 32-bit integer SCORE
// thresholds. CMiniGameAim::ResolveEntries (minigame_aim_and_party_tiers.cpp) uses a loaded table or
// sign-extends three configured signed-short party targets into fallback slots
// [2]=tier0,[1]=tier1,[0]=tier2. Slots encode alternative targets, not XYZ.
// Zero is skipped by selection; all zero falls back to zero. Negative words are
// nonzero and therefore selectable, not automatically disabled. No validated
// positive range exists for loaded values. Preserve sums/differences within s32
// (including abs(totalTarget-totalScore), which must not be INT_MIN).
// Darts selects forward [0,1,2]: values are integer points in the same domain as
// holeScores/totalScore (ring points, hole-in=50, miss=-10); ranking subtracts
// score from target and excludes negative deviations. Drive selects [2,1,0]:
// values are integer targets for the distance-derived contest score (world XZ
// distance / 3.2, plus separately accumulated bonus), not raw world distances.
// The dialog stores the integer carrier and publishes it to text-table entry9
// via UITextTable_SetInteger (darts_dialog_and_load_data_menu.cpp); no pointer dereference or unit scaling.
// These units follow the consumers; do not rescale shared party thresholds or
// confuse a score target with the darts board's geometric radius.
Obj40::Obj40(void* resource) {
    void* values = ((CMiniGameAim*)resource)->GetEntry(0);
    void* items[3];
    items[0] = ((void**)values)[2];
    items[1] = ((void**)values)[1];
    items[2] = *(void**)values;
    void* selected;
    // On exhaustion use the first REVERSED slot (original slot 2, also zero).
    // Fallback stays in the exhaustion edge to preserve CodeWarrior's branch layout.
    for (int i = 0; i < kTargetValueCount || (selected = items[0], false); ++i) {
        if (items[i] != 0) {
            selected = items[i];
            break;
        }
    }
    __ct__12CDartsDialogFPvPv(this, selected, 1);
}

struct DartsValues {
    int items[3];
};
struct DartsVec {
    float x, y, z;
    // Copy XYZ as three unchanged words, preserving each float bit pattern.
    void operator=(const DartsVec& other) {
        *(DartsValues*)this = *(const DartsValues*)&other;
    }
};
struct DartsObject {
    char data[0x40];
    // Construct a darts result dialog using the first nonzero target value.
    DartsObject(DartsValues* values) {
        DartsValues copy = *values;
        int chosen;
        // First nonzero slot wins; exhausting all three uses original slot zero.
        for (int i = 0; i < kTargetValueCount || (chosen = values->items[0], false); ++i) {
            if (copy.items[i] != 0) {
                chosen = copy.items[i];
                break;
            }
        }
        __ct__12CDartsDialogFPvPv(this, (void*)chosen, 0);
    }
};
// Select the current hole's aim entry, falling back to hole zero.
inline DartsValues* DartsResource(void* self) {
    CSharedDoc* global = g_sharedDoc;
    void* player = (*(void**)&AsCSharedDoc(global)->golfDoc);
    int index = AsCGolfDoc(player)->currentHoleIndex;
    if (index < 0 || (unsigned int)index >= kHoleCount)
        index = 0;
    return (DartsValues*)((CMiniGameAim*)(AsCDartsSystem(self)->aimTable))
        ->GetEntry(AsCDartsSystem(self)->holeEntryIds[index]);
}
// Borrowed synchronous request ABI (state values in DartsRequests/DriveRequests):
// darts 0: index -> output(int target); 1: output(int shot score); 2: update ranks;
// 3: output(float[3] aim), unchanged if no aim; 4: output(int within radius);
// 5: output(int dialog availability sampled BEFORE possible creation).
// drive 0: output(float distance), secondaryOutput(float hole-in bonus);
// 1: update ranks; 2: output(int dialog ready BEFORE possible creation).
// Each case requires the relevant writable storage; no payload pointer is retained.
struct DartsInfo {
    int unused;
    int state;
    void* output;
    union {
        int index;
        void* secondaryOutput;
    };
};
typedef char DartsInfo_size[(sizeof(DartsInfo) == 0x10) ? 1 : -1];
struct DartsDistanceTier {
    int score;
    float radius;
};
struct DartsPlayerScoreView {
    char prefix[0x17C];
    int holeScores[18];
    int totalScore;
};
// Vertex stream: stride 0x18 = XYZ float[3] at +00, packed color at +0C,
// UV float[2] at +10. Descriptor words are buffer handle, vertex count.
// Both target and plate passes consume triangle strips in this layout.
struct DartsVertexStream {
    unsigned int buffer;
    unsigned int vertexCount;
};
struct DartsDelete {
    virtual ~DartsDelete();
};

// Answer darts queries for target values, shot scores, ranks, aim position, and dialog state.
// self/info required; request-specific outputs are described by DartsInfo above.
// state is a request opcode, not the actor's persistent state. info->unused is
// ignored. Unknown opcodes leave all fields/outputs unchanged. Distance and
// radius use world XZ units. The ordinary world-label consumer CSpot::DrawMarkersAndLabels
// (spot_marker_draw_and_messages.cpp) formats horizontal distance/3.200000047683716 as yards; hence a
// radius R here corresponds to R/3.2 in the game's yard convention. CDartsSystem
// does NOT perform that division: keep table radii/ball/target in the same world
// space. To specify a new radius in game yards, store yards*3.2 in the data in a
// separate intentional mod. This establishes the game convention, not SI scale.
// Scoring: hole-in = 50; otherwise horizontal distance selects the FIRST tier with
// distance <= radius (ring boundaries included). Outside all four tiers = -10.
// Relative angle = atan2(ballDelta.x, ballDelta.z)-targetAngle, wrapped to [0,2pi).
// The closed sector [0,pi/2] scores zero; the rest uses the selected tier score.
void CDartsSystem::HandleMsg(const DartsInfo* info) {
    DartsVec position;
    DartsVec pos;
    DartsValues vec;
    DartsVec direction;
    DartsVec delta;
    DartsVec from;
    DartsVec delta4;
    DartsVec from4;
    switch (info->state) {
    case kDartsGetHoleTarget: {
        int index = info->index;
        if (index < 0 || (unsigned int)index >= kHoleCount)
            index = 0;
        DartsValues* values = (DartsValues*)((CMiniGameAim*)aimTable)->GetEntry(holeEntryIds[index]);
        vec = *values;
        int chosen;
        int i = 0;
        int* item = vec.items;
        // Same selection policy as the dialog: scan copied values, then on
        // exhaustion read the fallback from the original entry's first slot.
        for (; i < kTargetValueCount || (chosen = values->items[0], false); ++item, ++i)
            if (*item != 0) {
                chosen = vec.items[i];
                break;
            }
        *(int*)info->output = chosen;
        break;
    }
    case kDartsScoreCurrentShot: {
        float angle = heading;
        position = (*(DartsVec*)&this->position[0]);
        void* manager = GolfBall__Fv();
        int* mgr = (int*)manager;
        int result;
        int a = ((int*)((CGolfBall*)mgr)->m_holeInCrypt)[0];
        int b = ((int*)((CGolfBall*)mgr)->m_holeInCrypt)[2];
        if ((a ^ ~b) == 1)
            result = kHoleInScore;
        else {
            Vec3_CopyToOutput(GolfBall__Fv(), &from);
            Vec3_Subtract(&from, &from, &position);
            from.y = kFloat_0_darts;
            delta = from;
            float distance = Vec3_Length(&delta);
            const DartsDistanceTier* thresholds = (const DartsDistanceTier*)dartsDistanceScoreTiers;
            unsigned int tier;
            for (tier = 0; tier < kDistanceTierCount; ++tier) {
                if (distance <= thresholds[tier].radius)
                    break;
            }
            if (tier < kDistanceTierCount) {
                int x = ((int*)&from)[0];
                ((int*)&direction)[0] = x;
                int z = ((int*)&from)[2];
                int y = ((int*)&from)[1];
                ((int*)&direction)[2] = z;
                float dx = direction.x;
                float dz = direction.z;
                ((int*)&direction)[1] = y;
                float diff = (float)atan2(dx, dz) - angle;
                while (diff >= kFloat_6_2831855)
                    diff -= kFloat_6_2831855;
                while (diff < kFloat_0_darts)
                    diff += kFloat_6_2831855;
                int inside;
                if (kFloat_0_darts <= diff && diff <= kFloat_1_5707964_tweenDartsAudio)
                    inside = 1;
                else
                    inside = 0;
                if (inside)
                    result = 0;
                else
                    result = ((const DartsDistanceTier*)dartsDistanceScoreTiers)[tier].score;
            } else
                result = kMissScore;
        }
        *(int*)info->output = result;
        break;
    }
    case kDartsUpdateRanks:
        UpdatePlayerRanks((void*)info);
        break;
    case kDartsGetAimPosition:
        if (hasAimPosition)
            Vec3_CopySourceToDestination(aimPosition, info->output);
        break;
    case kDartsCheckScoringRadius: {
        pos = (*(DartsVec*)&this->position[0]);
        void* manager = GolfBall__Fv();
        Vec3_CopyToOutput(manager, &from4);
        Vec3_Subtract(&from4, &from4, &pos);
        from4.y = kFloat_0_darts;
        delta4 = from4;
        float distance = Vec3_Length(&delta4);
        unsigned int tier;
        const DartsDistanceTier* thresholds = (const DartsDistanceTier*)dartsDistanceScoreTiers;
        for (tier = 0; tier < kDistanceTierCount; ++tier) {
            if (distance <= thresholds[tier].radius)
                break;
        }
        *(int*)info->output = tier < kDistanceTierCount;
        break;
    }
    case kDartsEnsureResultDialog: {
        int done = 0;
        if (dialogRequested == 0 || dialog != 0)
            done = 1;
        *(int*)info->output = done;
        if ((*(void**)&dialogRequested) == 0) {
            void* old = dialog;
            dialogRequested = 1;
            if (old) {
                delete (DartsDelete*)old;
                dialog = 0;
            }
            DartsObject* object = new DartsObject(DartsResource(this));
            dialog = object;
        }
        break;
    }
    }
}

#pragma push
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
// Answer Driving Contest distance/rank/dialog queries using the borrowed DartsInfo ABI.
// Request 0 needs two distinct writable float outputs; both are zeroed FIRST.
// Recovery-required or nonzero lie returns with zeros. Otherwise output is
// horizontal displacement in CONTEST SCORE UNITS: one score unit is
// 3.200000047683716 world-coordinate units before the hundredths truncation.
// Concrete caller CGolfRule::ProcessBallFlight (golf_rule_round_flow_and_shot_sim.cpp) passes player+0x174
// for this shot's float score and a separate bonus float. It accumulates bonus
// into +0x170 and adds shot+bonus into +0x178 per-stroke results. HUD
// DrawFlightCarrySummary (golf_hud_flight_info_and_pang_rewards.cpp) prints these as contest values with its
// pool's p unit label; ranking (drive_system_distance_ranks.cpp) compares them as float scores, and
// PartyGame_EvaluatePlayerThresholdTier compares configured targets directly.
// This establishes an editable score/world conversion, independent of a claim
// about SI units or literal real-world travel. The same 3.2 scale also appears
// in ordinary golf yardage, but do not replace contest score formatting with
// yardage formatting or rescale its 100-point bonus implicitly.
// secondaryOutput is 100 score units only for decoded hole-in==1, else zero.
// Preserve current-minus-initial sample order, Y=0 and truncation to hundredths;
// this is displacement, not accumulated flight length. No gate multiplier here.
// Request 1 delegates ranking: REQUIRE 1<=current golf document playerCount<=4,
// all player slots alive; CDriveSystem::UpdateDistanceRanks uses fixed four-slot
// scratch arrays. Request 2 writes readiness BEFORE creation,
// creates once if not requested, and retains dialog until OnProcess/OnDestroy.
// Unknown states do nothing; pointers are not validated or retained by queries.
// Optimizer pragmas preserve the original load/temporary lifetimes and FP order.
void CDriveSystem::HandleMsg(const DartsInfo* info) {
    int state = info->state;
    switch (state) {
    case kDriveUpdateRanks:
        UpdateDistanceRanks();
        break;
    case kDriveGetDistance: {
        *(float*)info->output = kFloat_0_NumericMenuInput;
        *(float*)info->secondaryOutput = kFloat_0_NumericMenuInput;
        void* manager = GolfBall__Fv();
        if (((CGolfBall*)manager)->m_recoveryRequired != 0)
            return;
        manager = GolfBall__Fv();
        if (((CGolfBall*)manager)->m_lie != 0)
            return;
        // Initial trajectory sample and current ball share an XYZ prefix.
        // Ignore height; divide world horizontal distance by the display divisor,
        // then truncate (do not round) the result to two decimal places.
        void* initialSample = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
        manager = GolfBall__Fv();
        float displacement[3];
        __mi__FRC7WVectorRC7WVector(displacement, manager, initialSample);
        displacement[1] = kFloat_0_NumericMenuInput;
        float horizontalWorldDistance = (*(WVector*)displacement).Magnitude();
        float displayDistance = horizontalWorldDistance / kDriveWorldUnitsPerScoreUnit;
        int wholeDistance = (int)displayDistance;
        float fractionalDistance = displayDistance - (float)wholeDistance;
        int truncatedHundredths = (int)(kFloat_100_NumericMenuInput * fractionalDistance);
        float truncatedDisplayDistance =
            (float)wholeDistance + (float)truncatedHundredths / kFloat_100_NumericMenuInput;
        *(float*)info->output = truncatedDisplayDistance;
        manager = GolfBall__Fv();
        {
            int holeInCheckWord = ((int*)((CGolfBall*)manager)->m_holeInCrypt)[2];
            int holeInValueWord = ((int*)((CGolfBall*)manager)->m_holeInCrypt)[0];
            int decodedHoleIn = ~(holeInValueWord ^ holeInCheckWord);
            if (decodedHoleIn == 1)
                *(float*)info->secondaryOutput = kFloat_100_NumericMenuInput;
        }
    } break;
    case kDriveEnsureResultDialog: {
        int ready = dialogRequested != 0 && (*(int*)&dialog) != 0;
        *(int*)info->output = ready;
        if (dialogRequested != 0)
            return;
        dialogRequested = 1;
        void* old = dialog;
        if (old != 0) {
            if (old != 0) {
                ((VObj0*)old)->call(1);
            }
            dialog = 0;
        }
        Obj40* object = new Obj40(miniGameAim);
        dialog = object;
    } break;
    default:
        break;
    }
}
#pragma pop

// Release the drive result dialog and mini-game aim resource.
void CDriveSystem::OnDestroy() {
    if (dialog != 0) {
        if (dialog != 0) {
            struct VObj0 {
                virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            VObj0* p = (VObj0*)dialog;
            p->c(1);
        }
        dialog = 0;
    }
    if (miniGameAim != 0) {
        if (miniGameAim != 0) {
            ((VObj4*)miniGameAim)->call(1);
        }
        miniGameAim = 0;
    }
}

// Shared preset kDartsTexturedRenderStateDefaults at .rodata 0x8048CCF0:
// +00 render pairs (ID,value), in application order:
//   (2,0) lighting off; (19,1) ambient color source=vertex;
//   (18,1) material color source=vertex; (20,1) cached ID20;
//   (9,0) cached ID9; (0,0) GX_CULL_NONE; (10,1) blending enabled;
//   (7,1) alpha test enabled; (4,4) GX_GREATER; (11,0) alpha reference byte0;
//   (13,4) blend source=source alpha; (14,5) destination=1-source alpha;
//   (6,1) depth test on; (5,1) depth writes on; (3,3) GX_LEQUAL.
// Consumer map: Engine_SetCachedRenderState (gx_renderer_state_and_efb_copy.cpp), Render_ApplyLightingChannels,
// Render_ApplyAlphaTestState/DepthState/BlendState (gx_render_state_and_texture_upload.cpp). IDs9/20 have no
// identified value reader in these draw paths; cached words at engine+0x100/
// +0x12C stay0/1. ID20 DOES dirty lighting state in the setter even though the
// inspected lighting consumer does not test that word. Preserve that side effect;
// do not invent a lighting equation for20. ID9 has no dirty-group case.
// Blend operation/logic override are inherited (not present in this preset).
// +78 texture-stage pairs: (0,0),(1,2),(2,0),(3,0),(4,2),(5,0).
// IDs0/3 multiply color/alpha; IDs1/4 choose texture (2), IDs2/5 previous (0).
// Thus texture*previous RGBA, per TEV consumers gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp.
// +A8 sampler parameters: (4,1),(3,1),(5,1),(0,0),(1,0), translated by
// Engine_SetTextureStageParameter (gx_renderer_state_and_resources.cpp) to min/mag/mip/wrapS/wrapT.
// Min/mip=(1,1) resolves trilinear (one-level texture forces mip=0), mag1=linear,
// wraps0=GX_CLAMP. Supported wrap values0/1/2=clamp/repeat/mirror; mag0/1=nearest/
// linear. Other min/mip combinations must use s_textureMinMipFilterTable's
// actual pairs, not GX filter IDs. Parameter IDs are distinct from state IDs.
// This preset is shared by target AND plates: a later mod for only one pass
// needs separate settings. Copies own no resource; values are words, not floats.
// Applying this preset establishes the textured pass's state; later explicit
// overrides below are part of that pass and are not restored automatically.
struct TargetStateWords {
    unsigned int words[30];
};
struct TargetStream {
    unsigned int a, b, c, d, e, f, g, h, i, j, k, l;
};
struct TargetFormat {
    unsigned int a, b, c, d, e, f, g, h, i, j;
};
struct TargetGlobals {
    TargetStateWords state;
    TargetStream stream;
    TargetFormat format;
};
struct TargetLocal {
    unsigned char flag;
    virtual void a();
    virtual void apply();
};
struct TargetLocals {
    unsigned char local[8];
    TargetStream stream;
    TargetFormat format;
    TargetStateWords state;
};

#pragma push
#pragma use_lmw_stmw on
// Draw the darts target texture using its bound vertex stream.
// Installs identity transform slots 2/11 and the shared textured-state tables.
// Depth writes are disabled (state 5=0), depth compare LEQUAL (state 3=3).
// Unbinds texture at exit; does not restore prior transforms/render/vertex state.
// Resource and vertex-stream descriptor are borrowed for synchronous submission.
void CDartsTarget::Draw() {
    TargetGlobals* globals = (TargetGlobals*)kDartsTexturedRenderStateDefaults;
    if (renderSuppressed == 0) {
        SetTransform(2, kIdentityMatrix4x4);
        SetTransform(11, kIdentityMatrix4x4);
        TargetLocals s;
        *(void**)(s.local + 4) = s_texturedPrimitiveStateApplyVtable;
        s.local[0] = 0;
        s.state = globals->state;
        s.stream = globals->stream;
        s.format = globals->format;
        Engine_SetRenderStateList(&s.state, 15);
        Engine_SetTextureStageStateList(0, &s.stream, 6);
        Engine_SetTextureStageParameterList(0, &s.format, 5);
        Engine_SetType7Resource(0);
        ((TargetLocal*)s.local)->apply();
        Engine_SetCachedRenderState(kDartsDepthWriteState, kDartsDisableDepthWrite);
        Engine_SetCachedRenderState(kDartsDepthCompareState, kDartsCompareLessEqual);
        void* p = textureResource;
        void* q = vertexStreamInfo;
        if (p != 0) {
            Engine_SetTextureStageResource(0, ISharedResource_GetLocked(p));
            Engine_SetTexturePalette(0, ISharedResource_GetPaletteObject(p));
        } else {
            Engine_SetTextureStageResource(0, 0);
            Engine_SetTexturePalette(0, 0);
        }
        Engine_SetFlexibleVertexFormat(kDartsVertexFormat);
        Engine_SetVertexStream(0, ((DartsVertexStream*)q)->buffer, 0, kDartsVertexStride);
        DrawPrimitiveFromBoundBuffers(kDartsTriangleStrip, 0, ((DartsVertexStream*)q)->vertexCount);
        Engine_SetTextureStageResource(0, 0);
        Engine_SetTexturePalette(0, 0);
    }
}
#pragma pop

struct DartsRanks {
    CW_BITS2(unsigned char high : 4, unsigned char low : 4)
};
// Resolve the active golf document without caching it across player-entry calls.
static inline CGolfDoc* DartsGolfDocument() {
    return (CGolfDoc*)g_sharedDoc->golfDoc;
}
// rankFlags packs high rank in bits 7..4 and low rank in bits 3..0 on this ABI.
// Low is the shared competition placing; high additionally resolves equal results
// by rankTieBreakOrder (lower byte wins). Each starts at 1 and counts better players.
static inline DartsRanks& DartsPlayerRanks(void* player) {
    return *(DartsRanks*)&((GolfDocSnapshot8013*)player)->rankFlags;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Rank players by outright hole wins, total target deviation, then player order.
// self requires loaded aim data; shared golf doc has 1..4 live players and a
// current hole 0..17. Each player supplies scores through that hole. info is
// unused. Writes totalScore and both rank nibbles; all player/aim records are
// borrowed. Duplicate player lookups must retain their order (no cached alias).
// Entry/storage precondition is strict: 1<=playerCount<=4; zero is not an empty
// operation and >4 writes beyond outrightWins/holeRanks/totalDeviations below.
// For single player the source is a configured achievement tier, not opponents:
// TourPartyConfig_ApplyToSharedDoc (minigame_aim_and_party_tiers.cpp) clears partyResultTier and loads
// three signed-short {target,comparison,reward} slots from account/party config.
// PartyGame_EvaluatePlayerThresholdTier tests slots 3,2,1, returning highest
// satisfied enabled condition or 0. Score/count compare >= or == (codes4/5);
// distance uses float score >= target (code6). CGolfRule completion paths in
// golf_rule_round_flow_and_shot_sim.cpp publish that result into shared+0xAF0 in party gameMode6.
// This rank policy collapses tiers2/3 to first, tier1 to second,0/unlisted to
// third, writing BOTH rank nibbles; tier3 still differs from2 for reward lookup
// in minigame_aim_and_party_tiers.cpp. Tune achievement targets there, or this mapping for placing;
// do not replace a tier with a rank in shared storage. Test tiers0,1,2,3,
// multiplayer ties/overshoots and all four slots separately.
void CDartsSystem::UpdatePlayerRanks(void* /*info*/) {
    // The first pass totals recorded hole scores; the second counts unique closest-to-target wins.
    // Equal win counts compare total deviation, then the player-order byte for the high rank nibble.
    int* deviationCursor;
    int* winsCursor;
    int* otherWinsCursor;
    int* otherDeviationCursor;
    int holeScoreByteOffset;
    int* holeRankCursor;
    int targetScore;
    int otherPlayerIndex;
    int playerIndex;
    int holeIndex;
    int comparisonIndex;
    int count;
    int scoreOffsetBytes;
    int targetTotal;
    int targetDeviation;
    count = DartsGolfDocument()->playerCount;
    // Phase 1: sum each player's signed per-hole scores into totalScore.
    for (playerIndex = 0; playerIndex < count; ++playerIndex) {
        void* item = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
        ((DartsPlayerScoreView*)item)->totalScore = 0;
        comparisonIndex = 0;
        scoreOffsetBytes = 0;
        for (; comparisonIndex <= DartsGolfDocument()->currentHoleIndex; ++comparisonIndex) {
            void* a = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
            void* c = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
            int v = *(int*)((char*)((DartsPlayerScoreView*)a)->holeScores + scoreOffsetBytes);
            ((DartsPlayerScoreView*)c)->totalScore += v;
            scoreOffsetBytes += sizeof(int);
        }
    }
    if (count == 1) {
        int partyResultTier = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier);
        void* it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), 0);
        DartsPlayerRanks(it).high = 3;
        switch (partyResultTier) {
        case 2:
        case 3:
            it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), 0);
            DartsPlayerRanks(it).high = 1;
            break;
        case 1:
            it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), 0);
            DartsPlayerRanks(it).high = 2;
            break;
        }
        void* a = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), 0);
        void* c = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), 0);
        DartsPlayerRanks(c).low = DartsPlayerRanks(a).high;
    } else {
        int outrightWins[4];
        int holeRanks[4];
        int totalDeviations[4];
        DartsValues vec;
        // Phase 2: rank non-overshooting scores on each hole. Exactly one
        // rank-zero player earns a win; ties and overshoots earn none.
        targetTotal = 0;
        for (int i = 0; i < count; ++i)
            outrightWins[i] = 0;
        holeIndex = 0;
        holeScoreByteOffset = 0;
        for (; holeIndex <= DartsGolfDocument()->currentHoleIndex; ++holeIndex) {
            int idx = holeIndex;
            if (idx < 0 || (unsigned int)idx >= kHoleCount)
                idx = 0;
            DartsValues* values = (DartsValues*)((CMiniGameAim*)aimTable)->GetEntry(holeEntryIds[idx]);
            vec = *values;
            int i = 0;
            int* v = vec.items;
            // Select first nonzero target; exhaustion falls back to original
            // slot zero. The fallback runs only after all copied slots failed.
            for (; i < kTargetValueCount || (targetScore = values->items[0], false); ++v, ++i)
                if (*v != 0) {
                    targetScore = vec.items[i];
                    break;
                }
            targetTotal += targetScore;
            holeRankCursor = holeRanks;
            for (otherPlayerIndex = 0; otherPlayerIndex < count; ++holeRankCursor, ++otherPlayerIndex) {
                *holeRankCursor = -1;
                {
                    void* item = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)otherPlayerIndex);
                    void* p = (void*)(holeScoreByteOffset + (unsigned int)item);
                    int r0 = ((DartsPlayerScoreView*)p)->holeScores[0];
                    targetDeviation = targetScore - r0;
                }
                if (targetDeviation < 0)
                    continue;
                (*holeRankCursor)++;
                for (comparisonIndex = 0; comparisonIndex < count; ++comparisonIndex) {
                    void* item2 = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)comparisonIndex);
                    void* p2 = (void*)(holeScoreByteOffset + (unsigned int)item2);
                    int r02 = ((DartsPlayerScoreView*)p2)->holeScores[0];
                    int val2 = targetScore - r02;
                    if (otherPlayerIndex == comparisonIndex)
                        continue;
                    if (val2 < 0)
                        continue;
                    if (targetDeviation <= val2)
                        continue;
                    (*holeRankCursor)++;
                }
            }
            int additionalHoleWinners = -1;
            for (int i = 0; i < count; ++i)
                if (holeRanks[i] == 0)
                    additionalHoleWinners++;
            for (int i = 0; i < count; ++i) {
                if (holeRanks[i] == 0 && additionalHoleWinners == 0)
                    outrightWins[i]++;
            }
            holeScoreByteOffset += 4;
        }
        // Phase 3: compare aggregate deviation after counting all unique hole wins.
        int totalPlayerIndex = 0;
        for (; totalPlayerIndex < count; ++totalPlayerIndex) {
            void* item = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)totalPlayerIndex);
            int r0 = ((DartsPlayerScoreView*)item)->totalScore;
            totalDeviations[totalPlayerIndex] = abs(targetTotal - r0);
        }
        // Phase 4: write both packed placings; player-order ties affect only high.
        winsCursor = outrightWins;
        deviationCursor = totalDeviations;
        for (playerIndex = 0; playerIndex < count; ++winsCursor, ++deviationCursor, ++playerIndex) {
            DartsPlayerRanks(GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex)).high = 1;
            DartsPlayerRanks(GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex)).low = 1;
            otherWinsCursor = outrightWins;
            otherDeviationCursor = totalDeviations;
            for (otherPlayerIndex = 0; otherPlayerIndex < DartsGolfDocument()->playerCount;
                 ++otherWinsCursor, ++otherDeviationCursor, ++otherPlayerIndex) {
                if (playerIndex == otherPlayerIndex)
                    continue;
                int r0 = *otherWinsCursor;
                int playerWins = *winsCursor;
                if (playerWins < r0) {
                    void* it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                    DartsPlayerRanks(it).high++;
                    it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                    DartsPlayerRanks(it).low++;
                } else if (playerWins == r0) {
                    int otherDeviation = *otherDeviationCursor;
                    int playerDeviation = *deviationCursor;
                    if (playerDeviation > otherDeviation) {
                        void* it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                        DartsPlayerRanks(it).high++;
                        it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                        DartsPlayerRanks(it).low++;
                    } else if (playerDeviation == otherDeviation) {
                        void* it26 = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)otherPlayerIndex);
                        void* it25 = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                        unsigned char c26 = ((GolfDocSnapshot8013*)it26)->rankTieBreakOrder;
                        unsigned char c25 = ((GolfDocSnapshot8013*)it25)->rankTieBreakOrder;
                        if (c25 > c26) {
                            void* it = GolfPlayer_GetEntryByIndex(DartsGolfDocument(), (unsigned char)playerIndex);
                            DartsPlayerRanks(it).high++;
                        }
                    }
                }
            }
        }
    }
}
#pragma pop

// Same shared preset/pair layout as TargetGlobals; copied before local overrides.
struct PlateStateWords {
    unsigned int words[30];
};
struct PlateStream {
    unsigned int a, b, c, d, e, f, g, h, i, j, k, l;
};
struct PlateFormat {
    unsigned int a, b, c, d, e, f, g, h, i, j;
};
struct PlateGlobals {
    PlateStateWords state;
    PlateStream stream;
    PlateFormat format;
};
struct PlateReverse {
    unsigned int elementAddress;
    unsigned int* current;
};
extern unsigned char s_cameraTransform[0x100];
struct PlateLocal {
    unsigned char flag;
    virtual void a();
    virtual void apply();
};
struct PlateLocals {
    PlateReverse p, copy, stop, end, begin;
    unsigned char local[8];
    DartsVec position;
    int pad4C;
    DartsVec transformed;
    int pad5C;
    float matrix[16];
    PlateStream stream;
    PlateFormat format;
    PlateStateWords state;
};

// Draw the existing plate order forwards or backwards according to camera side.
// Camera world position minus plate origin is rotated by -heading into plate-local
// space. Negative local Z traverses the preordered descriptor vector backwards;
// nonnegative Z traverses forwards. This is a side test, not a general depth sort.
// Only fog-enable state 1 is saved/restored. Identity transforms and shared state lists
// remain installed; each draw unbinds texture. Caller owns state for the next pass.
void CDartsPlates::Draw() {
    PlateGlobals* globals = (PlateGlobals*)kDartsTexturedRenderStateDefaults;
    if (renderSuppressed != 0)
        return;

    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);

    PlateLocals s;
    *(void**)(s.local + 4) = s_texturedPrimitiveStateApplyVtable;
    s.local[0] = 0;
    s.state = globals->state;
    s.stream = globals->stream;
    s.format = globals->format;
    Engine_SetRenderStateList(&s.state, 15);
    Engine_SetTextureStageStateList(0, &s.stream, 6);
    Engine_SetTextureStageParameterList(0, &s.format, 5);
    Engine_SetType7Resource(0);
    ((PlateLocal*)s.local)->apply();
    void* savedFogEnabled;
    Engine_GetRenderState(kPlateFogEnabledState, &savedFogEnabled);
    Engine_SetCachedRenderState(kPlateFogEnabledState, 0);
    extern void Mtx44_SetRotationY(void*, float);
    extern void Vec3_Subtract(void*, void*, void*);
    extern void Mtx44_TransformPointToVec4(void*, void*, void*);
    Vec3_CopyToOutput(s_cameraTransform + 0x24, &s.position);
    Mtx44_SetRotationY(s.matrix, -heading);
    Vec3_Subtract(&s.position, &s.position, pad_004);
    Mtx44_TransformPointToVec4(&s.transformed, &s.position, s.matrix);
    s.position.x = s.transformed.x;
    s.position.y = s.transformed.y;
    s.position.z = s.transformed.z;
    void* v;
    // Each vector entry points to {bufferHandle, vertexCount}; pointer ownership
    // remains with the plates object. Both traversal branches bind identical state.
    unsigned int* rec;
    if (s.transformed.z < kFloat_0_darts) {
        s.end.current = (*(unsigned int**)&plates) + (*(int*)&plateCount);
        s.copy = s.end;
        s.p = s.copy;
        while ((s.begin.current = (*(unsigned int**)&plates), s.stop = s.begin, s.p.current != s.stop.current)) {
            v = (*(void* volatile*)&texture);
            s.p.elementAddress = (unsigned int)(((volatile PlateReverse*)&s.p)->current - 1);
            rec = (unsigned int*)*(unsigned int*)s.p.elementAddress;
            if (v) {
                Engine_SetTextureStageResource(0, ISharedResource_GetLocked(v));
                Engine_SetTexturePalette(0, ISharedResource_GetPaletteObject(v));
            } else {
                Engine_SetTextureStageResource(0, 0);
                Engine_SetTexturePalette(0, 0);
            }
            Engine_SetFlexibleVertexFormat(kDartsVertexFormat);
            Engine_SetVertexStream(0, rec[0], 0, kDartsVertexStride);
            DrawPrimitiveFromBoundBuffers(kDartsTriangleStrip, 0, rec[1]);
            Engine_SetTextureStageResource(0, 0);
            Engine_SetTexturePalette(0, 0);
            --s.p.current;
        }
    } else {
        unsigned int* p = (*(unsigned int**)&plates);
        while (p != (*(unsigned int**)&plates) + (*(int*)&plateCount)) {
            v = texture;
            rec = (unsigned int*)*p;
            if (v) {
                Engine_SetTextureStageResource(0, ISharedResource_GetLocked(v));
                Engine_SetTexturePalette(0, ISharedResource_GetPaletteObject(v));
            } else {
                Engine_SetTextureStageResource(0, 0);
                Engine_SetTexturePalette(0, 0);
            }
            Engine_SetFlexibleVertexFormat(kDartsVertexFormat);
            Engine_SetVertexStream(0, rec[0], 0, kDartsVertexStride);
            DrawPrimitiveFromBoundBuffers(kDartsTriangleStrip, 0, rec[1]);
            Engine_SetTextureStageResource(0, 0);
            Engine_SetTexturePalette(0, 0);
            ++p;
        }
    }
    Engine_SetCachedRenderState(kPlateFogEnabledState, (int)savedFogEnabled);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462570", _ctors_u802E2DD8_0, Darts_InitializeAngleConstant);
#pragma pop
