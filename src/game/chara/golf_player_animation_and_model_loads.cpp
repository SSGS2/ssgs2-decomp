// Golf player models and animation: per-player model and animation slots, pending model loads for
// story, swing and default models, the club trail effect, and showing or hiding the club.
// Also the shot follow-through reaction and the preparation animation.

#include "classes/GolfDocSnapshot8013.h"
#include "classes/GolfPlayerAnimationState.h"
// Borrow the animation-state ABI view; this conversion does not copy storage.
inline GolfPlayerAnimationState* AsGolfPlayerAnimationState(void* p) {
    return (GolfPlayerAnimationState*)p;
}
#include "iff/IffStruct.h"
#include "classes/GolfPlayerPendingNode.h"
#include "game/golfball.h"
// Borrow the ball ABI view used for landing state and pre-data indices.
inline CGolfBall* AsCGolfBall(void* p) {
    return (CGolfBall*)p;
}
#include "classes/CSharedDoc.h"
// Borrow the shared-document ABI view without changing ownership.
inline CSharedDoc* AsCSharedDoc(void* p) {
    return (CSharedDoc*)p;
}
#include "classes/CGolfPlayer.h"
// Borrow the player ABI view for model and animation storage.
inline CGolfPlayer* AsCGolfPlayer(void* p) {
    return (CGolfPlayer*)p;
}
#include "classes/CGolfDoc.h"
// Borrow the golf-document ABI view for active-player state.
template <class Address> inline CGolfDoc* AsCGolfDoc(Address p) {
    return (CGolfDoc*)p;
}

#include "game/singletons.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include "classes/CTailEffect.h"
extern "C" {

enum PlayerScriptConstants {
    kNoAnimationMode = 0,
    kPreparationAnimationMode = 2,
    kPowerShotAnimationMode = 3,
    kClubHiddenFlag = 0x80,
    kAttachmentDirtyFlag = 0x100,
    kScriptTokenCapacity = 0x40,
    kMaximumPartTokens = 0x40,
    kScriptCloseArguments = ')',
    kScriptAnimationNamePrefix = '@',
    kShotResultImmediateFlag = 0x40,
    kShotResultBallRecordFlag = 0x20000000,
    kRumbleDuration = 10,
    kWoodClub = 0,
    kIronClub = 1,
    kPutterClub = 3,
    kMinimumWoodIronReactionGrade = 2, // Grade names are not established here.
    kPlayerLongShotMessage = 0x1D,
    kGolfRulePuttReactionMessage = 0x18,
    kFirstRandomPuttReaction = 1
};

// Member offsets retained as byte addresses where typed addresses alter MWCC register lifetimes.
enum PlayerStorageOffset {
    kPendingModelTreeOffset = (int)&((CGolfPlayer*)0)->pendingModelLoadCount,
    kPendingNodeKeyOffset = (int)&((GolfPlayerPendingNode*)0)->playerIndex,
    kPendingNodeStringOffset = (int)&((GolfPlayerPendingNode*)0)->stringFlags
};

// The model manager's internals are opaque; array indexing uses its verified ABI size.
// Shared player records start after CSharedDoc's 0x10-byte prefix and have a 0x1D4 stride.
// Only the fields used by this unit are asserted here.
struct SharedPlayerSlotView {
    char pad_000[0x1BC];
    void* clubSetId;
    char pad_1C0[0xC];
    int swingModelVariant;
    char pad_1D0[4];
};
struct SharedPlayerArrayView {
    char pad_000[0x10];
    SharedPlayerSlotView players[4];
};
struct AttachmentCalibration {
    float ballDistance[3];
    float gripOffset;
};
struct PlayerAttachmentTablesView {
    char pad_000[0x128];
    AttachmentCalibration swing[4][4];
    AttachmentCalibration club[4];
};
typedef char SharedPlayerSlotView_size[(sizeof(SharedPlayerSlotView) == 0x1D4) ? 1 : -1];

struct PlayerModelSlot {
    char storage[0x2A0];
};
// Locate a player's model-manager slot using its verified array stride.
inline void* playerModelAt(void* player, int playerIndex) {
    return (PlayerModelSlot*)AsCGolfPlayer(player)->playerModels + playerIndex;
}
// Locate a player's animation state using the shared typed layout.
inline GolfPlayerAnimationState* playerAnimationAt(void* player, int playerIndex) {
    return (GolfPlayerAnimationState*)AsCGolfPlayer(player)->animationStates + playerIndex;
}
// InstanceTable_Get exposes the selected club kind and the current power-shot mode.
struct SelectedClubView {
    char pad_000[0xA];
    unsigned char kind;
};
struct PlayerInstanceView {
    char pad_000[4];
    SelectedClubView* selectedClub;
    char pad_008[0x3C];
    int powerShotMode;
};

extern char golfPlayerStrings[];
// Byte offsets in the shared Shift-JIS string pool.
enum GolfPlayerTextOffset {
    kPlayerTextClubSceneName = 0x29,
    kPlayerTextTail = 0x82,
    kPlayerTextRainbowTail = 0x87,
    kPlayerTextPinkTail = 0x94,
    kPlayerTextBlueTail = 0x9E,
    kPlayerTextStoryStandAnimation = 0xCC,
    kPlayerTextClubBone = 0x12F,
    kPlayerTextGolfRuleActor = 0x13A,
    kPlayerTextScreenActor = 0x143,
    kPlayerTextPowerShotReadySound = 0x14A,
    kPlayerTextPlayerActor = 0x240,
    kPlayerTextPuttFollowThroughAnimation = 0x247,
    kPlayerTextReadyEndSuffix = 0x5AA,
    kPlayerTextCaddieActor = 0x5B5,
    kPlayerTextStoryModelSuffix = 0x5BC,
    kPlayerTextSwingModelSuffix = 0x5C8,
    kPlayerTextModelSuffix = 0x5D4,
    kPlayerTextVoiceActor = 0x5DA,
};

extern char s_gameInputManager[];
extern char s_playerInputManagerDtorRegistration[];
extern void* g_sceneManager;
extern void* g_shadowManager;
extern CSharedDoc* g_sharedDoc;
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
extern float kFloat_0_5_GolfPlayer;
extern float kFloat_200_GolfSound;
extern float kFloat_1_GolfPlayer;
extern float kFloat_0_GolfPlayer;
extern float kLongWoodShotDistance_GolfPlayer;      // 416: long-shot horizontal distance threshold
extern float kPuttFollowThroughDistance_GolfPlayer; // 32: minimum initial putt distance for a reaction
extern float kFloat96_GolfPlayer;                   // 96: use four random putt reaction categories
extern float kFloat64_GolfPlayer;                   // 64: use three random putt reaction categories
extern float kShortPuttDistance_GolfPlayer;         // 3.2: near-pin distance for a putting reaction
extern float kPlayerInterpolationPi;                // pi, rounded in the original data
extern float kFloat_2_805DEA0C;
extern char kCaddieEnabledTipText[]; // Tip text shown when caddie handling is enabled.
void GolfTips_ShowIndexedMessage(int, const char*, int, float, float);
void GolfTipsDialog_RequestClose(int, float);
char* strcpy(char*, const char*);
char* strcat(char*, const char*);
void* ObjectList_FindAnimationRangeByName(void*, const char*, int, int);
void DrawScene_SetNamedObjectEnabled(void*, const char*, int);
void ShadowModel_SetNamedProjectionEnabled(void*, const char*, int);
void CGolfPlayer_SetPlayerAnimation(void*, const char*, float, int);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
void* GolfBall__Fv();
void InputManager_Init(void*);
void InputManager_StartRumble(void*, int);
void InputManager_Dtor();
int PuppetResourceList_PollLoadAndUpdateBounds(void*);
void RBTree_RebalanceAfterErase(void*, int);
void appFreeUni__FPv(void*);
void PlayerModelLoad_ReplacePendingRequest(void*, void*);
void IntStringTree_DestroySubtree(void*, void*);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
void* IntStringTree_FindInsertPosition(void*, int*, void**, unsigned char*, unsigned char*);
void* IntStringTree_InsertAtPosition(void*, void*, unsigned char, unsigned char, int*);
void* String_ReserveCapacity(void*, int);
void* ItemInventory_GetClubSetIdAt(void*, int);
void* InstanceTable_Get();
void* ClubModel_FindCachedForItem(void*, void*);
void* ObjectList_GetSelected(void*);
void* ItemInventory_FindCharacterData(void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* PreData__9CGolfBallFi(void*, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
int rand();

struct PendingTreeLink {
    PendingTreeLink* left;
    PendingTreeLink* right;
    unsigned int pc;
};
// Tree packet: +0x00 links/tagged parent, +0x0C player-model slot key,
// +0x10 owned path (12-byte MW string). Bit 31 selects heap storage at +0x18;
// otherwise the high byte contains the inline length and bytes start at +0x11.
// Polling clears the path before unlinking the packet; the node owns heap bytes.
struct PendingModelRequest {
    char pad_00[0xC];
    unsigned int index;
    union {
        unsigned int flag;
        struct {
            CW_BITS2(unsigned char heap : 1, unsigned char len : 7) char buf[3];
        } s;
    };
    char pad_14[4];
    char* ptr;
};
// Finish the current model load, start the next, or rebuild attachment data once all loads complete.
// Returns 0 while a request is loading or the next one has just started; 1 when
// no requests remain. Completed tree nodes and their owned model-path strings
// are destroyed here; models are kept in their separate per-player slots.
int CGolfPlayer::PollPendingModelLoads() {
    if (!(*(int*)&pendingModelLoadCount))
        return 1;
    PendingModelRequest* active = (*(PendingModelRequest**)&firstPendingModelLoad);
    if (!PuppetResourceList_PollLoadAndUpdateBounds((char*)playerModels + active->index * 0x2A0))
        return 0;
    // Phase 1: retire the completed path and unlink its request after caching
    // its successor. Parent low bit is a tree tag, never part of an address.
    if (!(active->flag >> 31)) {
        active->s.buf[0] = 0;
        active->s.len = 0;
    } else {
        *active->ptr = 0;
        (*(int*)&AsGolfPlayerPendingNode(active)->stringLength) = 0;
    }
    active = (*(PendingModelRequest**)&firstPendingModelLoad);
    {
        PendingTreeLink* node = (PendingTreeLink*)active;
        if (node == (PendingTreeLink*)active) {
            PendingTreeLink* successor = node->right;
            if (successor) {
                while (successor->left)
                    successor = successor->left;
                node = successor;
            } else {
                PendingTreeLink* parent;
                while (node != (parent = (PendingTreeLink*)(node->pc & ~1))->left)
                    node = parent;
                node = parent;
            }
            (*(PendingTreeLink**)&firstPendingModelLoad) = node;
        }
    }
    RBTree_RebalanceAfterErase(active, (*(int*)&pendingModelLoadRoot));
    if (&active->index && (char*)active + kPendingNodeStringOffset && (active->flag >> 31))
        appFreeUni__FPv(active->ptr);
    appFreeUni__FPv(active);
    int count = --(*(int*)&pendingModelLoadCount);
    // Phase 2: serialize loading; only the first remaining request is started.
    if (count) {
        void* nextRequest = firstPendingModelLoad;
        void* position = (char*)playerModels + (*(int*)&AsGolfPlayerPendingNode(nextRequest)->playerIndex) * 0x2A0;
        void* modelPath = !(AsGolfPlayerPendingNode(nextRequest)->stringFlags >> 31)
                              ? (char*)&AsGolfPlayerPendingNode(nextRequest)->stringFlags + 1
                              : (*(void**)&AsGolfPlayerPendingNode(nextRequest)->stringData);
        PlayerModelLoad_ReplacePendingRequest(position, modelPath);
        return 0;
    }
    // Phase 3: all models are ready; rebuild each player's swing/club placement
    // calibration, then recache animation key times for eligible player states.
    void* layout = ItemInventory_GetClubSetIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, 0); // original +0xA04
    void* object = ClubModel_FindCachedForItem(InstanceTable_Get(), layout);
    int modelOffset = 0, playerRecordOffset = 0;
    char* swingCalibration = (char*)((PlayerAttachmentTablesView*)this)->swing;
    char* clubCalibration = (char*)((PlayerAttachmentTablesView*)this)->club;
    for (int i = 0; i < AsCGolfDoc((void*)g_sharedDoc->golfDoc)->playerCount; ++i) {
        ((void (*)(void*, int, int, float, int))CGolfPlayer_SetPlayerAnimation)(
            this, (int)(golfPlayerStrings + kPlayerTextStoryStandAnimation), i, kFloat_0_GolfPlayer, 0);
        void* pos = ObjectList_GetSelected((char*)playerModels + modelOffset);
        void* asset = ClubModel_FindCachedForItem(InstanceTable_Get(),
                                                  *(void**)((char*)g_sharedDoc + playerRecordOffset + 0x1CC));
        void* item = ((CGolfDoc*)(((void*)g_sharedDoc->golfDoc)))->GetItemSelection(i);
        void* name =
            ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, *(void**)item); // original +0xA04
        for (int j = 0; j < 4; ++j)
            ReadClubSwingPlacement(pos, asset, name, j, swingCalibration + j * 0x10);
        ReadClubSwingPlacement(pos, object, name, 0, clubCalibration);
        modelOffset += 0x2A0;
        playerRecordOffset += 0x1D4;
        swingCalibration += 0x40;
        clubCalibration += 0x10;
    }
    for (int i = 0; i < AsCGolfDoc((void*)g_sharedDoc->golfDoc)->playerCount; ++i)
        if (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex((void*)g_sharedDoc->golfDoc, i))->playerState != 3)
            CacheShotKeyTimes(i);
    return 1;
}

struct EquippedClubSetView {
    char pad_00[0x1CC];
    unsigned int mode;
};
struct BallReactionRecord {
    char pad_00[0x70];
    int mode;
};
// Partial document view for the putting reaction; shared fields retain their header names.
struct ShotReactionDocument {
    char pad_000[0x2BC];
    void* holeRecords;
    char pad_2C0[0x14];
    int impactGrade;
    char pad_2D8[0xDE];
    unsigned char currentHoleIndex;
    char pad_3B7[9];
    char pad_3C0[0x40];
    int reactionActive;
    int reactionPending;
};
struct HolePinView {
    char pad_000[0xC];
    float x, y, z;
    char pad_018[8];
};
typedef char HolePinView_size[(sizeof(HolePinView) == 0x20) ? 1 : -1];
// Use the ball's horizontal distance from the pin to trigger long-shot/putting follow-through events.
// Wood/iron grades 0 or 1 return 0 without any reaction. Other paths return 1,
// including paths that emit nothing. enabled gates only the wood message.
// Distances use the ball/pin world-coordinate units; their physical scale is not
// established here. Y is vertical, so only X/Z contribute to reaction distances.
int CGolfPlayer::UpdateShotFollowThroughReaction(int enabled) {
    void* instanceTable = InstanceTable_Get();
    unsigned char clubKind = ((PlayerInstanceView*)instanceTable)->selectedClub->kind;
    if (clubKind == kWoodClub) {
        ShotReactionDocument* golfDoc = (ShotReactionDocument*)((void*)g_sharedDoc->golfDoc);
        if (golfDoc->impactGrade < kMinimumWoodIronReactionGrade)
            return 0;
    } else {
        instanceTable = InstanceTable_Get();
        clubKind = ((PlayerInstanceView*)instanceTable)->selectedClub->kind;
        if (clubKind == kIronClub) {
            ShotReactionDocument* golfDoc = (ShotReactionDocument*)((void*)g_sharedDoc->golfDoc);
            if (golfDoc->impactGrade < kMinimumWoodIronReactionGrade)
                return 0;
        }
    }
    ShotReactionDocument* golfDoc = (ShotReactionDocument*)((void*)g_sharedDoc->golfDoc);
    HolePinView* hole = (HolePinView*)((char*)golfDoc->holeRecords + golfDoc->currentHoleIndex * 0x20);
    // Snapshot the borrowed hole record's XYZ floats as raw words; these are
    // bitwise copies, not integer coordinates. WVector subtraction reads those
    // same 12 bytes. Ball records are borrowed from CGolfBall, never freed here.
    int pinPositionBits[3];
    pinPositionBits[0] = *(int*)&hole->x;
    pinPositionBits[1] = *(int*)&hole->y;
    pinPositionBits[2] = *(int*)&hole->z;
    void* ball = GolfBall__Fv();
    void* ballState = GolfBall__Fv();
    void* ballRecord = PreData__9CGolfBallFi(ballState, AsCGolfBall(ball)->m_preDataCount);
    // Current pre-data index supplies the final/current position; index 0 below
    // supplies the initial position. Both are measured against the same hole pin.
    float displacement[3], horizontalDisplacement[4];
    __mi__FRC7WVectorRC7WVector(displacement, ballRecord, pinPositionBits);
    *(WVector*)horizontalDisplacement = *(WVector*)displacement;
    horizontalDisplacement[1] = kFloat_0_GolfPlayer;
    float finalDistanceToPin = (*(WVector*)horizontalDisplacement).Magnitude();
    hole = (HolePinView*)((char*)golfDoc->holeRecords + golfDoc->currentHoleIndex * 0x20);
    pinPositionBits[0] = *(int*)&hole->x;
    pinPositionBits[1] = *(int*)&hole->y;
    pinPositionBits[2] = *(int*)&hole->z;
    ballRecord = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
    __mi__FRC7WVectorRC7WVector(displacement, ballRecord, pinPositionBits);
    *(WVector*)horizontalDisplacement = *(WVector*)displacement;
    horizontalDisplacement[1] = kFloat_0_GolfPlayer;
    float initialDistanceToPin = (*(WVector*)horizontalDisplacement).Magnitude();
    clubKind = ((PlayerInstanceView*)InstanceTable_Get())->selectedClub->kind;
    if (clubKind == kWoodClub) {
        // Wood: when recovery is not required and distance exceeds the threshold, send player message 0x1D.
        // The receiving actor's further behavior is outside this unit.
        if (enabled && !AsCGolfBall(GolfBall__Fv())->m_recoveryRequired &&
            finalDistanceToPin > kLongWoodShotDistance_GolfPlayer) // original +0x58
            IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextPlayerActor, kPlayerLongShotMessage, 0, 0,
                                   0, 0);
    } else if (clubKind == kPutterClub && initialDistanceToPin > kPuttFollowThroughDistance_GolfPlayer) {
        // Putt starting >32 from pin: choose 0..3 if >96, 0..2 if >64,
        // otherwise 0..1. Category 0 suppresses a NEW near-pin reaction; all
        // nonzero categories have the same outcome (not different animations).
        int category;
        if (initialDistanceToPin > kFloat96_GolfPlayer) {
            int randomValue = rand();
            category = randomValue % 4;
        } else if (initialDistanceToPin > kFloat64_GolfPlayer) {
            int randomValue = rand();
            category = randomValue % 3;
        } else {
            int randomValue = rand();
            category = randomValue % 2;
        }
        golfDoc = (ShotReactionDocument*)((void*)g_sharedDoc->golfDoc);
        if (!golfDoc->reactionActive) {
            // Inactive reaction: near pin (<3.2) and nonzero category sends the
            // rule message, queues the putt animation, and sets pending=1.
            // Otherwise clear pending. An active pending reaction reissues it
            // below without retesting distance or random category.
            if (finalDistanceToPin < kShortPuttDistance_GolfPlayer && category >= kFirstRandomPuttReaction) {
                IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextGolfRuleActor,
                                       kGolfRulePuttReactionMessage, 0, 0, 0, 0);
                strcpy(&playerAnimationAt(this, AsCGolfDoc(golfDoc)->currentPlayer)->pendingAnimationName,
                       golfPlayerStrings + kPlayerTextPuttFollowThroughAnimation);
                golfDoc->reactionPending = 1;
            } else
                golfDoc->reactionPending = 0;
        } else if (golfDoc->reactionPending) {
            IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextGolfRuleActor,
                                   kGolfRulePuttReactionMessage, 0, 0, 0, 0);
            strcpy(&playerAnimationAt(this, AsCGolfDoc(golfDoc)->currentPlayer)->pendingAnimationName,
                   golfPlayerStrings + kPlayerTextPuttFollowThroughAnimation);
        }
    }
    return 1;
}

// Interpolate three Euler angles along the short arc, wrapping differences larger than pi.
float* EulerAngles_LerpWrapped(void*, float fraction, float* result, const float* start, const float* end) {
    float endAngle;
    endAngle = end[0];
    float startAngle = start[0];
    float halfTurn = kPlayerInterpolationPi;
    float difference = startAngle - endAngle;
    if (difference > halfTurn || difference < -halfTurn) {
        float wrappedStart = *(const volatile float*)(start + 0) + halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        float wrappedEnd = *(const volatile float*)(end + 0) + halfTurn;
        if (wrappedEnd > halfTurn)
            wrappedEnd = wrappedEnd - kFloat_2_805DEA0C * halfTurn;
        wrappedStart = fraction * (wrappedEnd - wrappedStart) + wrappedStart;
        wrappedStart += halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        result[0] = wrappedStart;
    } else {
        result[0] = fraction * (endAngle - startAngle) + startAngle;
    }

    startAngle = start[1];
    endAngle = end[1];
    difference = startAngle - endAngle;
    if (difference > halfTurn || difference < -halfTurn) {
        float wrappedStart = *(const volatile float*)(1 + start) + halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        float wrappedEnd = *(const volatile float*)(end + 1) + halfTurn;
        if (wrappedEnd > halfTurn)
            wrappedEnd = wrappedEnd - kFloat_2_805DEA0C * halfTurn;
        wrappedStart = fraction * (wrappedEnd - wrappedStart) + wrappedStart;
        wrappedStart += halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        result[1] = wrappedStart;
    } else {
        result[1] = fraction * (endAngle - startAngle) + startAngle;
    }

    startAngle = start[2];
    endAngle = end[2];
    difference = startAngle - endAngle;
    if (difference > halfTurn || difference < -halfTurn) {
        float wrappedStart = *(const volatile float*)(start + 2) + halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        float wrappedEnd = *(const volatile float*)(end + 2) + halfTurn;
        if (wrappedEnd > halfTurn)
            wrappedEnd = wrappedEnd - kFloat_2_805DEA0C * halfTurn;
        wrappedStart = fraction * (wrappedEnd - wrappedStart) + wrappedStart;
        wrappedStart += halfTurn;
        if (wrappedStart > halfTurn)
            wrappedStart = wrappedStart - kFloat_2_805DEA0C * halfTurn;
        result[2] = wrappedStart;
    } else {
        result[2] = fraction * (endAngle - startAngle) + startAngle;
    }
    return result;
}

struct AnimationRangeView {
    float startFrame, endFrame;
    char pad_008[0x10];
    int sequenceId;
};

// Start the named preparation animation up to its final frame and queue its completion notification.
int CGolfPlayer::StartReadyEndPreparationAnimation(const char* name) {
    int result = 0;
    playerAnimationAt(this, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer)->animationMode = 2;
    playerAnimationAt(this, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer)->pendingAnimationName = 0;
    strcpy((char*)&swingAnimationName, name);
    char rangeName[0x80];
    strcpy(rangeName, name);
    CGolfDoc* active = AsCGolfDoc((void*)g_sharedDoc->golfDoc);
    char* rangeKey = strcat(rangeName, golfPlayerStrings + kPlayerTextReadyEndSuffix);
    float* range = (float*)ObjectList_FindAnimationRangeByName((char*)playerModels + active->currentPlayer * 0x2A0,
                                                               rangeKey, 1, 0);
    float oneFrame = kFloat_1_GolfPlayer;
    if (range[1] - range[0] > oneFrame) {
        animationCompletionNotifyPending = 1;
        ((void (*)(void*, int, float, int))CGolfPlayer_SetPlayerAnimation)(
            this, ((AnimationRangeView*)range)->sequenceId, range[1] - range[0] - oneFrame, -1);
        strcpy(&playerAnimationAt(this, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer)->pendingAnimationName,
               name);
        result = 1;
    } else
        animationCompletionNotifyPending = 0;
    return result;
}

// Toggle caddie handling and show or dismiss its tip according to the player's stored flag.
int CGolfPlayer::SetCaddieReactionMode() {
    if (*(unsigned char*)pad_064) {
        float f = kFloat_0_GolfPlayer;
        int result = 0;
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextScreenActor, 0x56, (int)&result, 0, 0, 0);
        if (!result)
            GolfTips_ShowIndexedMessage(5, kCaddieEnabledTipText, 0, f, kFloat_0_GolfPlayer);
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextCaddieActor, 0x13, 1, 0, 0, 0);
    } else {
        int result = 0;
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextScreenActor, 0x56, (int)&result, 0, 0, 0);
        if (!result)
            GolfTipsDialog_RequestClose(5, kFloat_0_GolfPlayer);
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextCaddieActor, 0x13, 0, 0, 0, 0);
    }
    return 1;
}

// Start a requested animation state and play the power-shot-ready sound for state three.
int CGolfPlayer::StartAnimationMode(int animationName, int animationMode, float startFrame) {
    playerAnimationAt(this, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer)->animationMode =
        animationMode; // original +0x3C1
    playerAnimationAt(this, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer)->pendingAnimationName = 0;
    ((void (*)(void*, int, float, int))CGolfPlayer_SetPlayerAnimation)(this, animationName, startFrame, -1);
    switch (animationMode) {
    case 3:
        Sound_PlayByName(g_soundManager, golfPlayerStrings + kPlayerTextPowerShotReadySound, 0, 0,
                         golfPlayerStrings + kPlayerTextPowerShotReadySound, 0, kFloat_0_5_GolfPlayer,
                         kFloat_200_GolfSound, kFloat_1_GolfPlayer);
        break;
    }
    return 1;
}

// Apply the shot result, request the rule transition, and rumble for an eligible first ball record.
void CGolfPlayer::PlayShotSoundsAndStartBallFlight(int unusedCommand, int shotResult) {
    PlaySwingAndTerrainSounds(shotResult);
    unsigned int flags =
        (unsigned int)AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags; // original +0x358
    if (flags & kShotResultImmediateFlag) {
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextGolfRuleActor, 7, 0x80, 0, 0, 0);
    } else if (flags & kShotResultBallRecordFlag) {
        BallReactionRecord* state = (BallReactionRecord*)((CGolfBall*)GolfBall__Fv())->GetOrAllocatePreDataAt(0);
        // A reaction already active in the document suppresses the rumble request.
        if (state->mode == 1 && AsCGolfDoc(((void*)g_sharedDoc->golfDoc))->shotReactionActive == 0) {
            if (GetInputManagerSlot() == 0) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_playerInputManagerDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            InputManager_StartRumble(GetInputManagerSlot(), kRumbleDuration);
        }
        IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextGolfRuleActor, 7, 0x80, 0, 0, 0);
    }
}

// Enable and clear the default/rainbow/pink/blue club trail selected by the equipped item type.
void CGolfPlayer::StartTailEffect() {
    CSharedDoc* manager = g_sharedDoc;
    CGolfDoc* active = (CGolfDoc*)manager->golfDoc;
    EquippedClubSetView* entry = (EquippedClubSetView*)((char*)manager + active->currentPlayer * 0x1D4);
    unsigned int mode = entry->mode & 0x03FFFFFF;
    switch (mode) {
    case 9:
        trailEnabled[1] = 1;
        ((CTailEffect*)(tailEffects[1]))->ClearTrail();
        DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextRainbowTail, 1);
        break;
    case 12:
        trailEnabled[2] = 1;
        ((CTailEffect*)(tailEffects[2]))->ClearTrail();
        DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextPinkTail, 1);
        break;
    case 10:
        trailEnabled[3] = 1;
        ((CTailEffect*)(tailEffects[3]))->ClearTrail();
        DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextBlueTail, 1);
        break;
    default:
        trailEnabled[0] = 1;
        ((CTailEffect*)(tailEffects[0]))->ClearTrail();
        DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextTail, 1);
        break;
    }
    IActor_DispatchMessage(this, this, golfPlayerStrings + kPlayerTextVoiceActor, 1, 0, 0, 0, 0);
}

// Unlock attachment-position updates for subsequent bone transforms.
void CGolfPlayer::vfunc_36() {
    attachmentPositionLocked = 0;
}

// Clear the player's club-hide flag and enable the club scene/shadow entries.
void CGolfPlayer::ShowClub(int, int index) {
    GolfPlayerAnimationState* entry = playerAnimationAt(this, index);
    entry->flags &= ~kClubHiddenFlag;
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextClubSceneName, 1);
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, golfPlayerStrings + kPlayerTextClubBone, 1);
}

// Set the player's club-hide flag and disable the club scene/shadow entries.
void CGolfPlayer::HideClub(int, int index) {
    GolfPlayerAnimationState* entry = playerAnimationAt(this, index);
    entry->flags |= kClubHiddenFlag;
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kPlayerTextClubSceneName, 0);
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, golfPlayerStrings + kPlayerTextClubBone, 0);
}

struct OwnedStringTree {
    OwnedStringTree* child[2];
    char pad_08[4];
    unsigned int flag;
    char pad_10[4];
    void* owned;
};
// Release an owned string tree, including both child subtrees.
static void fn_80148_free(OwnedStringTree* p, void* owner) {
    if (!p)
        return;
    for (int i = 0; i < 2; i++) {
        OwnedStringTree* q = p->child[i];
        if (q) {
            if (q->child[0])
                IntStringTree_DestroySubtree((char*)owner + kPendingModelTreeOffset, q->child[0]);
            if (q->child[1])
                IntStringTree_DestroySubtree((char*)owner + kPendingModelTreeOffset, q->child[1]);
            if ((q->flag >> 31) && q->owned)
                appFreeUni__FPv(q->owned);
            appFreeUni__FPv(q);
        }
    }
    if ((p->flag >> 31) && p->owned)
        appFreeUni__FPv(p->owned);
    appFreeUni__FPv(p);
}
// Keep the original aligned allocator-byte copies and small-string ownership ABI.
struct __attribute__((aligned(4))) Fn80148_Alloc {
    // Copy the allocator tag byte used by the string-copy ABI.
    Fn80148_Alloc& operator=(const Fn80148_Alloc& o) {
        *(char*)this = *(const char*)&o;
        return *this;
    }
};
struct Fn80148_CXString {
    union {
        unsigned int flags;
        struct {
            CW_BITS2(unsigned char heap : 1, unsigned char len : 7) char buf[3];
        };
    };
    unsigned int size;
    char* ptr;
    // Initialize an empty small string.
    Fn80148_CXString() {
        flags = 0;
        size = 0;
        ptr = 0;
    }
    // Copy inline bytes or deep-copy the owned heap path selected by the tag.
    Fn80148_CXString(const Fn80148_CXString& o) {
        if (!(o.flags >> 31)) {
            flags = o.flags;
            size = o.size;
            ptr = o.ptr;
        } else {
            flags = 0;
            size = 0;
            ptr = 0;
            Fn80148_Alloc src;
            Fn80148_Alloc box;
            String_ReserveCapacity(this, 0);
            box = src;
            String_ReplaceRange(this, 0, 0, o.ptr, o.ptr + o.size, (char*)&box);
        }
    }
    // Free only heap-backed string storage.
    ~Fn80148_CXString() {
        if (flags >> 31)
            appFreeUni__FPv(ptr);
    }
    // Read the active representation's byte length.
    unsigned int length() const {
        return !(flags >> 31) ? len : size;
    }
    // Borrow writable bytes from the active string representation.
    char* data() {
        if (!(flags >> 31))
            return buf;
        return ptr;
    }
};
struct Fn80148_Key {
    int index;
    Fn80148_CXString str;
    // Pair a player index with an owned copy of its model path.
    Fn80148_Key(int n, const Fn80148_CXString& s) : index(n), str(s) {}
};
struct PendingStringNode {
    char pad_00[0xC];
    int index;
    Fn80148_CXString str;
};
struct PendingStringFlags {
    char pad_00[0x10];
    unsigned int flag;
};

// Release a pending node's owned path, then the node; children are already gone.
// Interior-address guards retain the compiler shape of nested member destructors.
static inline void ReleasePendingModelNode(void* node) {
    if ((char*)node + kPendingNodeKeyOffset && (char*)node + kPendingNodeStringOffset &&
        (((PendingStringFlags*)node)->flag >> 31)) {
        void* owned = (*(void**)&AsGolfPlayerPendingNode(node)->stringData);
        appFreeUni__FPv(owned);
    }
    appFreeUni__FPv(node);
}

// Destroy deeper children, then release one frontier packet and its owned path.
static inline void DestroyPendingModelFrontier(void* self, void* node) {
    void* childNode = AsGolfPlayerPendingNode(node)->left;
    if (childNode)
        IntStringTree_DestroySubtree((char*)self + kPendingModelTreeOffset, childNode);
    childNode = AsGolfPlayerPendingNode(node)->right;
    if (childNode)
        IntStringTree_DestroySubtree((char*)self + kPendingModelTreeOffset, childNode);
    ReleasePendingModelNode(node);
}

#pragma use_lmw_stmw on
// Replace the pending model-load tree with each player's story, swing, or default model path.
void CGolfPlayer::RebuildPendingModelLoads(int enabled) {
    // Phase 1: postorder-destroy the old request tree (children, heap string,
    // then node). The first three levels are expanded; deeper branches delegate
    // to IntStringTree_DestroySubtree. No model-manager slot is destroyed here.
    void* leafNode;
    void* branchNode;
    void* subtreeNode;
    void* rootNode;
    rootNode = pendingModelLoadRoot;
    if (rootNode) {
        subtreeNode = AsGolfPlayerPendingNode(rootNode)->left;
        void* childNode;
        unsigned int unusedFlags;
        if (subtreeNode) {
            branchNode = AsGolfPlayerPendingNode(subtreeNode)->left;
            if (branchNode) {
                leafNode = AsGolfPlayerPendingNode(branchNode)->left;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                leafNode = AsGolfPlayerPendingNode(branchNode)->right;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                ReleasePendingModelNode(branchNode);
            }
            branchNode = AsGolfPlayerPendingNode(subtreeNode)->right;
            if (branchNode) {
                leafNode = AsGolfPlayerPendingNode(branchNode)->left;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                leafNode = AsGolfPlayerPendingNode(branchNode)->right;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                ReleasePendingModelNode(branchNode);
            }
            ReleasePendingModelNode(subtreeNode);
        }
        subtreeNode = AsGolfPlayerPendingNode(rootNode)->right;
        if (subtreeNode) {
            branchNode = AsGolfPlayerPendingNode(subtreeNode)->left;
            if (branchNode) {
                leafNode = AsGolfPlayerPendingNode(branchNode)->left;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                leafNode = AsGolfPlayerPendingNode(branchNode)->right;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                ReleasePendingModelNode(branchNode);
            }
            branchNode = AsGolfPlayerPendingNode(subtreeNode)->right;
            if (branchNode) {
                leafNode = AsGolfPlayerPendingNode(branchNode)->left;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                leafNode = AsGolfPlayerPendingNode(branchNode)->right;
                if (leafNode)
                    DestroyPendingModelFrontier(this, leafNode);
                ReleasePendingModelNode(branchNode);
            }
            ReleasePendingModelNode(subtreeNode);
        }
        ReleasePendingModelNode(rootNode);
        pendingModelLoadCount = 0;
        pendingModelLoadRoot = 0;
        firstPendingModelLoad = &pendingModelLoadRoot;
    }
    // Phase 2: tree is empty with firstPendingModelLoad at its embedded sentinel.
    // Build a path per player: enabled -> story; otherwise variant 0 -> swing;
    // otherwise the default model suffix. The tree owns a copy of each path.
    Fn80148_CXString modelPath;
    int count;
    void* foundNode;
    Fn80148_Alloc src1;
    Fn80148_Alloc box1;
    Fn80148_Alloc src2;
    Fn80148_Alloc box2;
    Fn80148_Alloc src3;
    Fn80148_Alloc box3;
    Fn80148_Alloc src4;
    Fn80148_Alloc box4;
    unsigned char insertHint;
    unsigned char insertSide;
    for (count = 0; count < AsCGolfDoc(((void*)g_sharedDoc->golfDoc))->playerCount; count++) {
        void* item = ((CGolfDoc*)(((void*)g_sharedDoc->golfDoc)))->GetItemSelection((unsigned char)count);
        char* characterData =
            (char*)ItemInventory_FindCharacterData((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, *(void**)item);
        unsigned int n = modelPath.length();
        box1 = src1;
        String_ReplaceRange(&modelPath, 0, n, characterData + IFF_OFFSET(IFF_STRUCT::sChar, model),
                            characterData + IFF_OFFSET(IFF_STRUCT::sChar, model) +
                                strlen(characterData + IFF_OFFSET(IFF_STRUCT::sChar, model)),
                            (char*)&box1);
        if (enabled) {
            char* suffix = golfPlayerStrings + kPlayerTextStoryModelSuffix;
            n = modelPath.length();
            box2 = src2;
            String_ReplaceRange(&modelPath, n, 0, suffix, suffix + strlen(suffix), (char*)&box2);
        } else if (((SharedPlayerArrayView*)g_sharedDoc)->players[count].swingModelVariant == 0) {
            char* suffix = golfPlayerStrings + kPlayerTextSwingModelSuffix;
            n = modelPath.length();
            box3 = src3;
            String_ReplaceRange(&modelPath, n, 0, suffix, suffix + strlen(suffix), (char*)&box3);
        } else {
            char* suffix = golfPlayerStrings + kPlayerTextModelSuffix;
            n = modelPath.length();
            box4 = src4;
            String_ReplaceRange(&modelPath, n, 0, suffix, suffix + strlen(suffix), (char*)&box4);
        }
        // Locate/insert by player index, then copy into the node's string payload.
        // The original computes the tree address separately for the two calls, so
        // the second call spells it as a byte offset from the player.
        leafNode =
            IntStringTree_FindInsertPosition(&pendingModelLoadCount, &count, &foundNode, &insertSide, &insertHint);
        if (foundNode && ((Fn80148_Key*)(branchNode = (char*)foundNode + kPendingNodeKeyOffset))->index >= count) {
        } else {
            Fn80148_CXString key;
            Fn80148_Key k(count, key);
            branchNode = (char*)IntStringTree_InsertAtPosition((char*)this + kPendingModelTreeOffset, leafNode,
                                                               insertSide, insertHint, &k.index) +
                         0xC;
        }
        Fn80148_CXString* destination = &((Fn80148_Key*)branchNode)->str;
        unsigned int destinationOnHeap = destination->flags >> 31;
        unsigned int sourceFlags;
        if (!destinationOnHeap && !((sourceFlags = modelPath.flags) >> 31)) {
            unsigned int sourceSize = modelPath.size;
            destination->flags = sourceFlags;
            destination->size = sourceSize;
            destination->ptr = modelPath.ptr;
        } else {
            unsigned int destinationLength = destinationOnHeap ? destination->size : destination->len;
            char* src;
            unsigned int sourceLength;
            Fn80148_Alloc src6;
            Fn80148_Alloc box6;
            if (!(modelPath.flags >> 31)) {
                src = modelPath.buf;
                sourceLength = modelPath.len;
            } else {
                src = modelPath.ptr;
                sourceLength = modelPath.size;
            }
            box6 = src6;
            String_ReplaceRange(destination, 0, (int)destinationLength, src, src + sourceLength, (char*)&box6);
        }
    }
    // Phase 3: start only the first request; PollPendingModelLoads chains the rest.
    // Existing precondition: at least one player/request must have been inserted.
    PendingStringNode* active = (PendingStringNode*)firstPendingModelLoad;
    void* pos = (char*)playerModels + active->index * 0x2A0;
    char* value = !(active->str.flags >> 31) ? active->str.buf : active->str.ptr;
    PlayerModelLoad_ReplacePendingRequest(pos, value);
}

} // extern "C"
