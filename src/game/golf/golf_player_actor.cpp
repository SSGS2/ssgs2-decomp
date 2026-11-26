// Golf player actor (CGolfPlayer): the player models, swing and pose animation, club and item
// attachments, the shot-gauge and ready animations, and the messages that drive them.
// Cutscene player slots and avatar loads are handled here too.

#include <decomp/offset_of.h>
#include <decomp/local_class.h>
extern "C" char golfPlayerInverseBasisMatrix[];
extern "C" char s_renderPacketDtorRegistration_golfPlayer[];
#include "iff/IffStruct.h"
// Field offsets are checked in the class headers; original function addresses stay below.
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CGolfPlayer.h"
#define AsCGolfPlayer(p) ((CGolfPlayer*)(p))
#include "classes/GolfPlayerAnimationState.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/WPuppet.h"
#include "classes/GameInputManager.h"
#include "classes/RenderPacketFields.h"
#include "classes/CSharedDoc.h"
#include "classes/CPreloadAvatarAsync.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/itemwindow.h"
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CTailEffect.h"
#include "classes/CQuadTree.h"
#include "classes/CHeadIcon.h"
#include "classes/CString.h"
#include "classes/CFx.h"
#include "classes/CShadowManager.h"

// The player actor's task callbacks and message handler: per-player model
// owners, animation records and pose records, and the cached swing placements.
//
// OnPreLoadInit and OnLoad build the trails, models and sound banks; OnInit
// allocates the poses. OnProcess advances animations and trails, OnDisplay
// applies cutscene slot commands, and HandleMsg serves the messages.
// Element sizes of the three per-player arrays (see the layout note above).
enum GolfPlayerStrides {
    kPlayerModelStride = 0x2A0,
    kAnimationStateStride = 0xB4,
    kPlayerPoseStride = 0x64
};
// gameStateFlags (CGolfDoc +0x358) bits tested in this unit; only the first has an established meaning.
enum GolfStateFlagMasks {
    kGameStateScriptedPose = 0x01000000,       // scripted state: consume pending slot/position/heading (see OnDisplay)
    kGameStateTerrainContactMask = 0x040A3000, // other modes that place the body from the terrain-contact frame
    kGameStateFlag4000 =
        0x4000 // third placement mode in CGolfPlayer::UpdatePlayerModelForDisplay (meaning not established)
};
enum GolfPlayerLimits {
    kPlayerSlotCount = 4,
    kSwingAnimationCount = 16,
    kTransformBufferCount = 5
};

// Shared-document equipment slots contain IFF IDs, separate from pose/model arrays.
struct PlayerEquipmentSlotView {
    char unknown00[0x1CC];
    unsigned int clubItemId;
    char unknown1D0[4];
};
typedef char PlayerEquipmentSlotSize[(sizeof(PlayerEquipmentSlotView) == 0x1D4) ? 1 : -1];
typedef char PlayerClubItemOffset[((int)DECOMP_OFFSETOF(PlayerEquipmentSlotView, clubItemId) == 0x1CC) ? 1 : -1];
// The cached club bundle has one borrowed puppet per ClubCategory, following its first word.
struct PlayerClubPuppetBundleView {
    void* unknown00;
    void* categoryPuppets[4];
};
typedef char PlayerClubPuppetsOffset[((int)DECOMP_OFFSETOF(PlayerClubPuppetBundleView, categoryPuppets) == 4) ? 1 : -1];

struct U80146020_Manager;
// Byte offsets into golfPlayerStrings (CP932 strings, including their terminators).
enum GolfPlayerTextOffset {
    kTextBallDist = 0x0,                                 // *ball_dist
    kTextF = 0xB,                                        // %f
    kTextGripPos = 0xE,                                  // *grip_pos
    kTextBall = 0x18,                                    // Ball
    kTextBallTarget = 0x1D,                              // Ball_Target
    kTextClubActor = 0x29,                               // \u30af\u30e9\u30d6
    kTextSwinglight01Gsr = 0x30,                         // ~swinglight01.gsr
    kTextSwinglight02Gsr = 0x42,                         // ~swinglight02.gsr
    kTextSwinglight01PinkGsr = 0x54,                     // ~swinglight01_pink.gsr
    kTextSwinglight01BlueGsr = 0x6B,                     // ~swinglight01_blue.gsr
    kTextTail = 0x82,                                    // tail
    kTextTailRainbow = 0x87,                             // tail_rainbow
    kTextTailPink = 0x94,                                // tail_pink
    kTextTailBlue = 0x9E,                                // tail_blue
    kTextPLAYERD = 0xA8,                                 // PLAYER_%d
    kTextStoryConte = 0xB2,                              // StoryConte
    kTextEntranceAnimation = 0xBD,                       // \u767b\u5834\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextMTSTORYSTAND = 0xCC,                            // MT_STORY_STAND
    kTextChar = 0xDB,                                    // char
    kTextSun = 0xE0,                                     // sun
    kTextVectorLengthError = 0xE4,                       // vector length error
    kTextStdExceptionStdLogicErrorStdLengthError = 0xF8, // !std::exception!!std::logic_error!!std::length_error!!
    kTextClub = 0x12F,                                   // club
    kTextShot = 0x134,                                   // *shot
    kTextGolfRule = 0x13A,                               // GolfRule
    kTextScreen = 0x143,                                 // Screen
    kTextPowershotReady = 0x14A,                         // powershot_ready
    kTextPowershotCancel = 0x15A,                        // powershot_cancel
    kTextSpot = 0x16B,                                   // Spot
    kTextCamera = 0x170,                                 // Camera
    kTextReadyFormat = 0x177,                            // %s\u6e96\u5099
    kTextPowerReadyEndFormat = 0x17E,                    // %s\u30d1\u30ef\u30fc\u6e96\u5099\u306e\u679c\u3066
    kTextDoublePowerReadyEndFormat = 0x191,              // %s\u30d1\u30ef\u30fc2\u6e96\u5099\u306e\u679c\u3066
    kTextMTSTORYADDITION05 = 0x1A5,                      // MT_STORY_ADDITION_05
    kTextBip01Head = 0x1BA,                              // Bip01 Head
    kTextSpr = 0x1C5,                                    // spr
    kTextSeq = 0x1C9,                                    // seq
    kTextBreathe = 0x1CD,                                // Breathe
    kTextBreathSeq = 0x1D5,                              // breath.seq
    kTextFirstPlaceAnimation = 0x1E0,                    // 1\u7b49\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextThirdPlaceAnimation = 0x1EE,                    // 3\u7b49\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextSecondPlaceAnimation = 0x1FC,                   // 2\u7b49\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextMTFACIALANIMATION = 0x20A,                      // MT_FACIAL_ANIMATION
    kTextPowerSidestepFormat = 0x21E,                    // %s\u87f9\u884c\u30d1\u30ef\u30fc
    kTextDoublePowerSidestepFormat = 0x22B,              // %s\u87f9\u884c\u30d1\u30ef\u30fc2
    kTextSidestepFormat = 0x239,                         // %s\u87f9\u884c
    kTextPlayer = 0x240,                                 // Player
    kTextPuttingFollowThrough = 0x247,                   // \u30d1\u30c3\u30c6\u30a3\u30f3\u30b0\u5f8c\u4ee3\u671f
    kTextClubNear = 0x25A,                               // club_near
    kTextClubFar = 0x264,                                // club_far
    kTextDoublePowerPrefix = 0x26D,                      // \u30d1\u30ef\u30fc2
    kTextPowerPrefix = 0x275,                            // \u30d1\u30ef\u30fc
    kTextEarlyPrefix = 0x27C,                            // \u65e9\u3044
    kTextLatePrefix = 0x281,                             // \u9045\u3044
    kTextSwingNameFormat = 0x286,                        // %s%s%s\u30b9\u30a4\u30f3\u30b0
    kTextPowerSwingFormat = 0x295,                       // %s\u30d1\u30ef\u30fc
    kTextShortPutt = 0x29E,                              // \u77ed\u3044\u30d1\u30c3\u30c6\u30a3\u30f3\u30b0
    kTextDoublePowerSwingFormat = 0x2AF,                 // %s\u30d1\u30ef\u30fc2
    kTextInvalidReadyAnimationWarning =
        0x2B9, // \u6e96\u5099\u52d5\u4f5c\u30a2\u30cb\u30e1\u30fc\u30b7\u30e7\u30f3\u306e\u4fee\u6b63\u304c\u5fc5\u8981\u3067\u3059.
    kTextWarningTitle = 0x2E1,      // \u8b66\u544a
    kTextCaddie = 0x2E6,            // caddie
    kTextRankedSpawnFormat = 0x2ED, // %d\u4eba_%d\u7b49
    kTextBone01 = 0x2F7,            // Bone01
    kTextHead = 0x2FE,              // Head
    kTextCharD = 0x303,             // char%d
    kTextPLAYER0 = 0x30A,           // PLAYER_0
    kTextWoodPowerFollowThrough =
        0x313, // \u30a6\u30c3\u30c9\u30b7\u30e7\u30c3\u30c8\u30d1\u30ef\u30fc\u5f8c\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextWoodFollowThrough = 0x334,    // \u30a6\u30c3\u30c9\u30b7\u30e7\u30c3\u30c8\u5f8c\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextDS = 0x34F,                   // %d - %s
    kTextRankAnimationSuffix = 0x357,  // \u7b49\u30e2\u30fc\u30b7\u30e7\u30f3
    kTextWoodShot = 0x364,             // \u30a6\u30c3\u30c9\u30b7\u30e7\u30c3\u30c8
    kTextIronShot = 0x373,             // \u30a2\u30a4\u30a2\u30f3\u30b7\u30e7\u30c3\u30c8
    kTextSandWedgeShot = 0x384,        // \u30b5\u30f3\u30c9\u30a6\u30a7\u30c3\u30b8\u30b7\u30e7\u30c3\u30c8
    kTextPuttingShot = 0x39B,          // \u30d1\u30c3\u30c6\u30a3\u30f3\u30b0\u30b7\u30e7\u30c3\u30c8
    kTextSwingSuffix = 0x3B0,          // \u30b9\u30a4\u30f3\u30b0
    kTextEarlySwingSuffix = 0x3B9,     // \u65e9\u3044\u30b9\u30a4\u30f3\u30b0
    kTextSwingVain = 0x3C6,            // swing_vain
    kTextSwingMissWood = 0x3D1,        // swing_miss_wood
    kTextSwingMissIron = 0x3E1,        // swing_miss_iron
    kTextGallery = 0x3F1,              // Gallery
    kTextSwingPowershotEffect = 0x3F9, // swing_powershot_effect
    kTextSwingPretenseWoodSmall = 0x410,                 // swing_pretense_wood_small
    kTextSwingPretenseWoodMiddle = 0x42A,                // swing_pretense_wood_middle
    kTextSwingPretenseWoodLarge = 0x445,                 // swing_pretense_wood_large
    kTextSwingTeeS = 0x45F,                              // swing_tee_s
    kTextSwingTeeW = 0x46B,                              // swing_tee_w
    kTextSwingDriveS = 0x477,                            // swing_drive_s
    kTextSwingDriveW = 0x485,                            // swing_drive_w
    kTextPicohanWood = 0x493,                            // picohan_wood
    kTextSwingTomahawk = 0x4A0,                          // swing_tomahawk
    kTextFlyTomahawk = 0x4AF,                            // fly_tomahawk
    kTextSwingCobra = 0x4BC,                             // swing_cobra
    kTextFlyCobra = 0x4C8,                               // fly_cobra
    kTextSwingNormalS = 0x4D2,                           // swing_normal_s
    kTextSwingNormalW = 0x4E1,                           // swing_normal_w
    kTextPicohanIron = 0x4F0,                            // picohan_iron
    kTextIronFairwaySoundFormat = 0x4FD,                 // \u30a2\u30a4\u30a2\u30f3_fairway%d
    kTextIronBunkerSoundFormat = 0x510,                  // \u30a2\u30a4\u30a2\u30f3_\u7802\u5834%d
    kTextBunkerSandEffect = 0x520,                       // \u30d0\u30f3\u30ab\u30fc\u7802\u624b\u629c\u304d
    kTextIronLeavesSoundFormat = 0x531,                  // \u30a2\u30a4\u30a2\u30f3_\u6728\u306e\u8449%d
    kTextSwingPutting = 0x543,                           // swing_putting
    kTextMogura = 0x551,                                 // mogura
    kTextHarisenPutt = 0x558,                            // harisen_putt
    kTextBound = 0x565,                                  // bound
    kTextCenter = 0x56B,                                 // center
    kTextDefaultAnimationFormat = 0x572,                 // %s\u30c7\u30d5\u30a9\u30eb\u30c8
    kTextLFoot = 0x57F,                                  // L Foot
    kTextRFoot = 0x586,                                  // R Foot
    kTextIgrass05Dds = 0x58D,                            // igrass05.dds
    kTextIgrass0401Dds = 0x59A,                          // igrass04_01.dds
    kTextReadyEndSuffix = 0x5AA,                         // \u6e96\u5099\u306e\u679c\u3066
    kTextCaddieActor = 0x5B5,                            // Caddie
    kTextStoryApet = 0x5BC,                              // .story.apet
    kTextSwingApet = 0x5C8,                              // .swing.apet
    kTextApet = 0x5D4,                                   // .apet
    kTextVoice = 0x5DA,                                  // Voice
    kTextMpet = 0x5E0,                                   // .mpet
    kTextFFF = 0x5E6,                                    // %f %f %f
    kTextPlayUnplayable = 0x5EF,                         // play_unplayable
    kTextDoublePowerReadyFormat = 0x5FF,                 // %s\u30d1\u30ef\u30fc2\u6e96\u5099
    kTextPowerReadyFormat = 0x60D,                       // %s\u30d1\u30ef\u30fc\u6e96\u5099
    kTextTreeInsertLengthError = 0x61A,                  // tree::insert length error
    kTextBasicStringReserveLengthError = 0x634,          // basic_string::reserve length_error
    kTextBasicStringOutOfRange = 0x657,                  // basic_string: out_of_range
    kTextStdExceptionStdLogicErrorStdOutOfRange = 0x672, // !std::exception!!std::logic_error!!std::out_of_range!!
    kTextBasicStringLengthError = 0x6A9,                 // basic_string: length_error
    kTextCGolfPlayer = 0x6C4,                            // CGolfPlayer
};

// Local views fill gaps in the shared class headers without changing those headers.
// The 0x64-byte pose record is separate from the 0xB4-byte animation record.
struct PlayerPoseState {
    int enabled;
    int turnedLastFrame;
    int turnedThisFrame;
    float headingRadians;
    float contactMatrix[12];
    Vec3f positionOverride;
    unsigned int positionOverrideEnabled;
    float headRotation;
    void* headBone;
    float groundSlopeRadians;
    void* footBones[2];
};
// Affine scratch stores basis vectors followed by translation, all in world coordinates.
struct PlayerAffineFrameView {
    Vec3f right, up, forward, position;
};
// Keep the original byte-array alignment while naming the matrix's extent/layout.
typedef char PlayerAffineScratch[sizeof(PlayerAffineFrameView)];
// Terrain-contact storage uses an extra 12-byte span before its world position;
// it must not be substituted for the packed affine view above.
struct PlayerContactFrameView {
    Vec3f right, up, forward;
    char span24[12];
    Vec3f position;
    unsigned int positionOverrideEnabled;
};
struct PlayerAnimationRecord {
    float time; // seconds in the selected range, advanced by OnProcess(deltaSeconds)
    void* animationObject;
    float modelMatrix[12];
    unsigned char visible;
    char pendingAnimationName[0x23];
    unsigned int flags;
    int mode;
    float shotKeyTimesSeconds[16]; // legacy name: elapsed seconds to *shot, not *ball_dist
    void* animationSource;
    void* appliedAnimationSource;
    float opacity;
    float opacityStep;
};
// One placement vector/scalar tuple per club category and player, derived from
// *ball_dist/*grip_pos keys. The scalar is added along the attachment's first
// basis vector; gripAxisOffset is a translation along that axis.
struct PlayerSwingPlacementView {
    char padToSwingName[0xA5];
    char swingAnimationName[0x80];
    char padToPlacements[3];
    SwingPlacement byPlayerAndClub[4][4];
    SwingPlacement defaultByPlayer[4];
};
struct SelectedClubView {
    char padToCategory[0xA];
    unsigned char category;
};
struct ShotInstanceView {
    void* vtable;
    SelectedClubView* selectedClub;
    char padToPowerMode[0x3C];
    int powerShotMode;
    char padToFlags[4];
    unsigned int flags;
};
typedef char PlayerPoseStateSize[(sizeof(PlayerPoseState) == 0x64) ? 1 : -1];
typedef char PlayerAnimationRecordSize[(sizeof(PlayerAnimationRecord) == 0xB4) ? 1 : -1];
typedef char AnimationFlagsAgree[((int)DECOMP_OFFSETOF(PlayerAnimationRecord, flags) ==
                                  (int)DECOMP_OFFSETOF(GolfPlayerAnimationState, flags))
                                     ? 1
                                     : -1];
typedef char AnimationModeAgree[((int)DECOMP_OFFSETOF(PlayerAnimationRecord, mode) ==
                                 (int)DECOMP_OFFSETOF(GolfPlayerAnimationState, animationMode))
                                    ? 1
                                    : -1];
typedef char SwingPlacementOffset[((int)DECOMP_OFFSETOF(PlayerSwingPlacementView, byPlayerAndClub) == 0x128) ? 1 : -1];

enum ClubCategory {
    kClubWood,
    kClubIron,
    kClubSandWedge,
    kClubPutter
};
enum PlayerAnimationMode {
    kAnimationReady = 0,
    kAnimationSwinging = 2,
    kAnimationEnteringPowerReady = 3,
    kAnimationPowerReady = 4
};
enum GolfPlayerInputAction {
    kActionTogglePlayerTranslucency = 0x22,
    kAnyInputPlayer = -1
};
enum GolfPlayerMessageId {
    kPlayerStartSwing = 0,
    kPlayerResetShotAnimation = 1,
    kPlayerTurnByAngle = 2,
    kPlayerGetHeading = 3,
    kPlayerGetGripOffset = 4,
    kPlayerGetSwingContactResult = 5,
    kPlayerPrepareTurn = 6,
    kPlayerFindBone = 7,
    kPlayerGetGroundSlope = 9,
    kPlayerSetTranslucency = 10,
    kPlayerTestSwingDirections = 11,
    kPlayerPrepareSwing = 12,
    kPlayerChangeEquippedItem = 14,
    kPlayerShow = 15,
    kPlayerHide = 16,
    kPlayerSetChildState = 17,
    kPlayerPlaceResults = 18,
    kPlayerGetAnimationMode = 21,
    kPlayerReloadPreview = 22,
    kPlayerSelectClub = 23,
    kPlayerSetPowerReady = 24,
    kPlayerSetPowerMode = 25,
    kPlayerPlaceWinner = 26,
    kPlayerPlacePreview = 27,
    kPlayerPlaceTurnPlayer = 28,
    kPlayerQueueFollowThrough = 29,
    kPlayerGetHeadMatrix = 30,
    kPlayerSetClubVisible = 31,
    kPlayerGetShotKeyTime = 32,
    kPlayerListAnimations = 33,
    kPlayerSelectAnimation = 34,
    kPlayerGetPosition = 35,
    kPlayerReleaseResources = 36,
    kPlayerReleaseModel = 37,
    kPlayerBeginAvatarLoad = 38,
    kPlayerIsAvatarLoadComplete = 39,
    kPlayerApplyAvatarLoad = 40,
    kPlayerHasModel = 41,
    kPlayerRefreshDisplay = 42,
    kPlayerSuspendProcessing = 43,
    kPlayerRemoveResultShadows = 44,
    kPlayerCreateShotShadows = 45,
    kPlayerBeginCutsceneSlots = 46,
    kPlayerEndCutsceneSlots = 47,
    kPlayerBuildPendingLoads = 48,
    kPlayerPollPendingLoads = 49,
    kPlayerSetPuppetState = 50,
    kPlayerClearTrails = 51,
    kPlayerCheckSpecialVictory = 52,
    kPlayerSetAnimation = 10005,
    kPlayerQueueAnimation = 10006,
    kPlayerSetHeading = 10007,
    kPlayerSetPosition = 10008
};
enum PlayerAnimationFlags {
    kAnimationTransformDirty = 0x1,
    kAnimationPlaying = 0x2,
    kAnimationAttachmentDirty = 0x8,
    kAnimationBodyLightDirty = 0x10,
    kAnimationClubLightDirty = 0x20,
    kAnimationSamplePose = 0x40,
    kAnimationClubHidden = 0x80,
    kAnimationUseLockedAttachment = 0x100,
    kAnimationPositionDirty = 0x200
};

// Affine matrices here store three basis vectors followed by translation (12 floats).
struct PlayerAttachmentAxesView {
    char padToAxes[0x288];
    Vec3f axisX, axisY, axisZ;
};
struct PlayerContactMatrix {
    Vec3f axisX, axisY, axisZ, position;
};
// The terrain contact search uses a separate position immediately AFTER the affine matrix.
struct SwingContactFrame {
    Vec3f axisX, axisY, axisZ, matrixTranslation, position;
};
struct PuppetBoneView {
    char padToFlags[0x74];
    unsigned int flags;
    PlayerContactMatrix worldMatrix;
};
// Named-bone lookups return a bone and its resource-space attachment position.
struct BoneAttachmentData {
    char prefix[0x18];
    Vec3f position;
};
struct BoneAttachmentEntry {
    PuppetBoneView* bone;
    BoneAttachmentData* attachment;
};
// This subobject is not yet exposed by ResourceModel801B's shared layout.
struct EquippedItemKeysView {
    char modelStorage[0x1BC];
    char equippedItemKeys[0xC];
};
struct BallHistoryView {
    char ballState[0x64];
    int activeHistoryIndex;
};
// Outfit data in each shared player block; resource loaders consume the following parameter block.
struct SharedOutfitSelection {
    unsigned int characterId;
    char selectionHeader[4];
    char partSelections[0x60];
    unsigned char packedOutfitSelection;
    char trailingSelectionStorage[7];
};
struct SharedPlayerSelectionView {
    char profilePrefix[0x44];
    void* rankValue;
    char profileStorage[0xD8];
    SharedOutfitSelection outfit;
    char avatarParameters[0x14];
};
typedef char SharedOutfitOffset[((int)DECOMP_OFFSETOF(SharedPlayerSelectionView, outfit) == 0x120) ? 1 : -1];
typedef char
    SharedAvatarParametersOffset[((int)DECOMP_OFFSETOF(SharedPlayerSelectionView, avatarParameters) == 0x190) ? 1 : -1];
enum {
    kClubPlacementArrayOffset = (int)DECOMP_OFFSETOF(PlayerSwingPlacementView, byPlayerAndClub),
    kClubPlacementBytes = sizeof(SwingPlacement),
    kPlayerPlacementBytes = sizeof(SwingPlacement) * 4,
    kBreathMatrixStorageOffset = (int)DECOMP_OFFSETOF(CGolfPlayer, slots_2E8),
    kBreathMatrixTranslationOffset = kBreathMatrixStorageOffset + (int)DECOMP_OFFSETOF(PlayerContactMatrix, position)
};
struct PuppetKeyRecord {
    int firstFrame, lastFrame;
    const char* text;
};
struct PlayerAnimationRange {
    float startTime, endTime;
    // These copied metadata values are not interpreted by this unit.
    float value08;
    unsigned int value0C;
    float value10;
    unsigned int value14;
    char* name;
    char* nextName;
    unsigned char flag20;
    char pad21[3];
    int firstKey, lastKey;
    unsigned short value2C, value2E, animationNameIndex;
    unsigned char flag32, flag33;
};
struct AnimationBlendBuffers {
    void* blended;
    void* horizontal;
    void* vertical;
    void* firstSample;
    void* secondSample;
};
struct SwingObstructionView {
    char padToObstruction[0x64];
    unsigned char swingPlacementBlocked;
};
typedef char PuppetBoneMatrixOffset[((int)DECOMP_OFFSETOF(PuppetBoneView, worldMatrix) == 0x78) ? 1 : -1];
typedef char
    AnimationNameIndexOffset[((int)DECOMP_OFFSETOF(PlayerAnimationRange, animationNameIndex) == 0x30) ? 1 : -1];
typedef char AnimationRangeSize[(sizeof(PlayerAnimationRange) == 0x34) ? 1 : -1];

// Additional layout contracts for fields that were previously raw offsets in this unit.
typedef char PosePositionOffset[((int)DECOMP_OFFSETOF(PlayerPoseState, positionOverride) == 0x40) ? 1 : -1];
typedef char PoseGroundSlopeOffset[((int)DECOMP_OFFSETOF(PlayerPoseState, groundSlopeRadians) == 0x58) ? 1 : -1];
typedef char SwingContactPositionOffset[((int)DECOMP_OFFSETOF(SwingContactFrame, position) == 0x30) ? 1 : -1];
typedef char BonePositionOffset[((int)DECOMP_OFFSETOF(PuppetBoneView, worldMatrix.position) == 0x9C) ? 1 : -1];
typedef char AttachmentAxisYOffset[((int)DECOMP_OFFSETOF(PlayerAttachmentAxesView, axisY) == 0x294) ? 1 : -1];
typedef char AttachmentAxisZOffset[((int)DECOMP_OFFSETOF(PlayerAttachmentAxesView, axisZ) == 0x2A0) ? 1 : -1];
typedef char InstancePowerModeOffset[((int)DECOMP_OFFSETOF(ShotInstanceView, powerShotMode) == 0x44) ? 1 : -1];
typedef char InstanceFlagsOffset[((int)DECOMP_OFFSETOF(ShotInstanceView, flags) == 0x4C) ? 1 : -1];

struct IntVector;
typedef Vec3f TargetVec;
struct ResourceString;
extern "C" {
#pragma auto_inline off
void* PlayerModel_GetEquippedItemKeyList(void* ptr);
int PuppetBone_IsAttachedEffectSuppressed(void* ptr);
unsigned int* Array_GetElementPtr(unsigned int** ptr, unsigned int index);
unsigned int* Array_GetElementPtr2(unsigned int** ptr, unsigned int index);
void AttachedSprayIds_Clear(void* ptr);
void AttachedSequenceIds_Clear(void* ptr);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void* WPuppet_FindNamedBoneEntry(void*, const char*);
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
void NamedEffectList_SetDistance(
    void*, const char*, float); // Takes an UNSQUARED distance and stores its square (draw_scene_named_effect_list.cpp).
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, const void*, const void*, const void*);
void* __nw__FUl(unsigned int);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned int);
char* String_FormatTemporary(const char*, ...);
void DrawScene_SetNamedObjectEnabled(void*, const char*, int); // Set the named scene entry's visibility.
void* GolfPlayerAnimationState_Ctor(void*);
void PlayerAnimationState_dtor();

void Matrix_SetIdentityAffine(void*);
void Sound_PlayByName(void*, const void*, int, int, int, int, float, float, float);
int IActor_DispatchMessage(void*, void*, const char*, int, unsigned int, int, int, int);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void Sound_PlayByNameWithParameters(void*, void*, void*, int, int, int, int, float, float, float);
void* InstanceTable_Get();
void InstanceTable_ClearCachedResources(void*);
void* ItemInventory_GetClubSetIdAt(void*, int);
void* ClubModel_CreateForItem(void*, unsigned int);
void* ItemInventory_FindCharacterData(void*, void*);
void* PlayerModel_GetAnimationResourceVector(void*);
void __destroy_new_array(void*, void*);
void PlayerModel_ClearState(void*);
void PlayerModel_Dtor(void*);
int AvatarModel_LoadCharacterResources(void*, void*, void*, int, int, int, int, int);
void ItemSelection_RefreshPrimaryItems(void*);
void UpdatePuppetBoundsForAnimationList(void*);
void* PlayerModel_GetPuppet(void*);                             // Get the player-model owner's primary puppet.
void DrawScene_SetNamedObjectPuppet(void*, const char*, void*); // Bind an object to the named scene entry.
extern "C" void* GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(void*, const char*, float, int, const char*,
                                                                         const void*);
extern "C" void
ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(void*, const char*,
                                                      void*); // Bind a puppet to a named shadow/effect group.
void CGolfPlayer_SetPlayerAnimation(CGolfPlayer*, const char*, float, int);
int SoundName_BuildAvatarPrefix(unsigned int, void*);
void SoundManager_AcquireSoundBank(void*, void*);
void* appMallocDummy__FUl(unsigned int);
void RenderPacket_Init(void*);
void RenderPacket_Destroy();
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv();
void Vec3_Zero(void*);
float* WPuppet_FindAnimationRangeByName(void*, const void*);
int WPuppet_FindKeyRange(void*, int, int, int*, int*);
void String_ScanTokensAfterSubstring(const char**, const char*, const char*, ...);
void Model_SetMatrixRecursive(void*, void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void* Matrix_TransformPoint(void*, const void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __ml__FRC7WVectorf(void*, const void*, float);
float* ObjectList_FindAnimationRangeByName(void*, const char*, int, int);
void* ObjectList_GetSelected(void*);
void* GolfBall__Fv();
int Input_IsActionPressed(void*, int, int);
void InputManager_Init(void*);
void InputManager_Dtor();
void Sound_StopByAlias(void*, const char*);
void GolfTask_NoOpStatus();
int ResourceModel801B_HasModelAndParts(void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, const void*, int, int);
void PlayerPoseState_Init(void*);
unsigned int Color_ScaleRGB(unsigned int, float);
void* __as__7WVectorFRC7WVector(void*, const void*);
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void WPuppet_UpdateSkinFromAnimationSource(void*, void*);
void WPuppet_UpdateBound(void*, int);
int WView_DefaultCapabilityTrueA(void*, void*);
void PlayerModel_ApplyBoneRelativeLighting(void*, void*, int, int);
void* WPuppet_FindOrCacheBone(void*, const void*);
void AvatarModel_DefaultReturnOneHook(void*); // Model interface success hook; this build has no side effects.
void* Matrix_CopyAffine(void*, const void*);
void __ml__FfRC7WVector(void*, const void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void WPuppet_UpdateBoundsFromBone(void*);
void CourseSlotTable_InitEmpty(const char*);
void SharedStringState_DefaultHook(int);
void* SlotTable_GetEntryByte(void*, int);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void* CQuadTree_GetInstance();
void Matrix_SetAxisRotation(void*, float, int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
int FadeState_GetState(void*);
void AttachedSprayIds_Append(IntVector*, unsigned int*);
void AttachedSequenceIds_Append(IntVector*, unsigned int*);
void* SharedDoc_GetCurrent();
void* ClubModel_FindCachedForItem(void*, void*); // Look up the club-model set by item ID in the instance table.
void* FxAttachment_Init(void*);
char* String_ParseQuotedToken(char*, char*);
char* String_ParseSpaceDelimitedToken(char*, char*);
void* String_ParseDecimalInt(char*);
void* ResolvedItemSelection_ContainsPart(void*, void*);
void* PuppetBone_GetWorldMatrix(void*);
int String_CompareCString(const void*, const void*); // Compare two NUL-terminated strings (zero means equal).
int SharedDoc_IsCourse8Or13();
void* WPuppet_FindNamedBoneEntryForward(void*, const char*);
void Vector_CopyThreeWords(unsigned int*, const unsigned int*);
void* Vec3_Set(void*, float, float, float);
void* Matrix_ConcatAffineAssign(void*, void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void WPuppet_SetRendererOption(void*, void*);
void* PreData__9CGolfBallFi(void*, unsigned int);
float Magnitude__7WVectorCFv(void*);
void ShadowModel_SetNamedProjectionEnabled(unsigned int, const char*,
                                           int); // Set the named shadow/effect group's enabled state.
void GfxObjList_RemoveEntryFromNamedGroup(unsigned int, const char*, void*);
void GfxObjList_RemoveGroupByName(unsigned int, const char*);
float Vector_SignedAngleXZ(void*, void*);
void MessageBox_Report(int, const char*, const char*, int);
int ItemInventory_FindPartData(void*, unsigned int);
int AvatarModel_ApplyValidatedItemChanges(void*, void*, unsigned int);
void WPuppet_SetHierarchyContextRecursive(void*, void*);
void* CourseData_FindNamedPoint(void*, const char*);
void WPuppet_ShowBoneSelection(void*, int);
void WPuppet_HideBoneSelection(void*, const char*);
void PuppetOwner_SetOpacityRecursive(void*, int);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, const void*, int, int, int);
void* WPuppet_FindBoneByName(void*, const void*);
void PlayerModel_SetDefaultPuppetLayerEnabled(void*, int);
unsigned char PlayerRank_FindTierForValue(void*);
void ItemSelection_BuildCharacterOutfit(void*, unsigned int, unsigned int, unsigned int, void*, unsigned char);
void* Object_GetChild(unsigned int);
int ItemInventory_FindCharacterIndex(void*, unsigned int);
unsigned char SharedDoc_GetCourseId();
void Matrix_AppendEulerRotation(void*, void*);
void PlayerModel_ReleaseResourcesByMask(void*, int);
float WVectorLen__FRC7WVector(void*);
void Normalize__7WVectorFv(void*);
void Vector_CrossProduct(void*, const void*, const void*);
float Math_SinApprox(float);
int CQuadTree_OverlappedWithObject(void*, const void*, float);
double asin(float);
void* InstanceTable_GetPowerShotModeWord(void*);
int ItemInventory_FindClubSetIndex(void*, int);
int SharedDoc_LoadEnemyPlayerSlotByName(
    int, int); /* (index, id): defined in story_event_script_shared_state.cpp as (int index, const char* name) */
void Matrix_ConcatAffine(void*, void*, void*);
void WPuppetBone_SetCurrentMatrix(void*, void*, int);
int SharedDoc_IsPlayerFlag0Set(int);
void WPuppet_SetOpacityRecursive(void*, int);
void WPuppet_GetBoneFlags(void*, int);
void __amu__7WVectorFf(void*, float);
int WPuppet_GetAnimationNameSize(void*, unsigned short);
void* WPuppet_CopyAnimationName(void*, void*, unsigned short, int);
void* WPuppet_GetAnimationValueByName(void*, void*, void*);
int WPuppet_EvaluateAnimationTransforms(void*, void*, int, void*, float);
int WPuppet_GetSnrtCount(void*);
int WPuppet_EvaluateTransforms(void*, void*, int, void*, void*, int);
int WPuppet_OverrideSkinMatricesForwarder(void*, void*, int);
void PSMTX44Identity(void*);
int WPuppet_SetAnimationDisabled(void*, int);
int WPuppet_GetFtCount(void*);
void WPuppet_EvaluateModelTransformAnimation(void*, void*);
int Vec3_Lerp(void*, void*, void*, float);
void Vector_CopyComponents(void*, void*);
int EulerAngles_LerpWrapped(void*, float, void*, void*, void*);
unsigned char InstanceTable_GetClubCategory(void*);
extern void* g_sceneManager;
extern const float kFloat_0_GolfPlayer;
extern const float kPlayerSceneDistance640; // 640: unsquared distance passed to NamedEffectList_SetDistance.
extern const float kFloat_0_05_PlayerShadow;
extern const float kFloat_1_GolfPlayer;
extern const float kFloat_0_5_GolfPlayer;
extern const float kFloat_200_GolfSound;
extern void* g_soundManager;
extern CSharedDoc* g_sharedDoc;
extern void* g_headIcon;
extern signed char s_renderPacketInitialized;
extern void* g_shadowManager;
extern char s_renderPacket[];
extern char s_renderPacketDtorRegistration_golfPlayer[];
extern char s_sharedCharDeleterVtable[];
extern char golfPlayerStrings[];
extern const char* s_swingAnimationNames[16];
extern const float kAnimationFramesPerSecond_GolfPlayer; // 30 animation frames per second.
extern const float kSwingPlacementDepth_GolfPlayer;      // -0.4 initial grip-placement depth.
extern const float kGolfPlayerNearZeroTolerance;         // 0.00001 animation-time tolerance.
extern float kIdleRestartDistance_GolfPlayer;            // 9.6 idle-animation restart distance.
extern signed char s_inputManagerInitialized;
extern GameInputManager s_gameInputManager;
extern char s_playerInputManagerDtorRegistration[];
extern char s_sceneLighting[];
extern void* g_view;
extern float kDegreesToRadians_GolfPlayer; // pi / 180: cutscene heading conversion.
extern float kFloat_16_0;
extern char s_conteModelSlots[0x20];
extern char s_fadeOverlay[0x20];
extern char s_fadeOverlayDtorRegistration_GolfPlayer[0x20]; // Fade-overlay destructor registration storage.
extern signed char s_courseSlotTableInitialized;
extern signed char s_fadeOverlayInitialized;
extern float kBreathEffectYaw_GolfPlayer; // -pi / 2: breath-effect orientation.
// Separate small-data symbols supply the character-effect tag prefix, including its terminator.
extern unsigned char s_characterEffectTagPrefixByte0;
extern unsigned char s_characterEffectTagPrefixByte1;
extern unsigned char s_characterEffectTagPrefixByte2;
extern unsigned char s_characterEffectTagPrefixByte3;
extern unsigned char s_characterEffectTagPrefixByte4;
// Separate small-data symbols supply the club-effect tag prefix, including its terminator.
extern unsigned char s_clubEffectTagPrefixByte0;
extern unsigned char s_clubEffectTagPrefixByte1;
extern unsigned char s_clubEffectTagPrefixByte2;
extern unsigned char s_clubEffectTagPrefixByte3;
extern unsigned int g_currentTimeMilliseconds;
extern unsigned int g_taskManager;
extern const float kMinimumGroundSlopeRadians_GolfPlayer; // -20 degrees in radians.
extern const float kMaximumGroundSlopeRadians_GolfPlayer; // 20 degrees in radians: slope clamp/clearance limit.
extern const float kSwingDirectionStepRadians_GolfPlayer; // pi / 4: eight candidate swing directions.
extern unsigned int g_resourceManager;
extern void* g_lastGolfTaskTypeProbeTarget;
extern unsigned int s_golfTaskTypeInfo;
extern int s_specialVictoryHoleIndices[12];
extern float kLongWoodShotDistance_GolfPlayer;      // 416: long wood-shot follow-through threshold.
extern float kPuttFollowThroughDistance_GolfPlayer; // 32: randomized putt follow-through threshold.
extern float kFloat96_GolfPlayer;                   // 96: largest randomized putt-distance bucket.
extern float kFloat64_GolfPlayer;                   // 64: shared putt-distance/opacity value.
extern float kShortPuttDistance_GolfPlayer;         // 3.2: short-putt animation distance.
extern float kShortPuttHeadingTolerance_GolfPlayer; // pi / 6: short-putt heading tolerance.
extern float kWinnerHeadingRadians_GolfPlayer;      // pi: default result-scene heading.
extern float kCoarseSwingContactStep_GolfPlayer;    // 0.1: coarse contact-search step.
extern float kFineSwingContactStep_GolfPlayer;      // 0.01: fine contact-search step.
extern float kSwingGripClearance_GolfPlayer;        // 1.6: grip-position bounds-test parameter.
extern float kGroundSlopeSign_GolfPlayer;           // -1: ground-slope sign convention (radians).
extern float kStrongSwingSoundPower_GolfPlayer;     // 0.9: strong/weak swing-sound threshold.
extern float kCaddieShadowParameter_GolfPlayer;     // 0.09: caddie shadow-entry parameter.
extern char golfPlayerBasisMatrix[];
extern char golfPlayerInverseBasisMatrix[];
extern float kGrassAttachmentDepth_GolfPlayer; // 0.32: lower attachments over selected grass textures.
extern float kOpaquePlayerAlpha_GolfPlayer;    // 255: fully opaque player/club alpha.
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];

// One volatile byte used as the allocator-tag argument of String_ReplaceRange (its value is a copy of the source tag).
struct StringAllocTag {
    union {
        volatile char value;
        unsigned int alignment;
    };
};
#pragma opt_propagation off
#pragma opt_common_subs off
// Local owning string (flags/length/data: bit 31 of flags = heap text) built only to pass a resource name to a constructor.
struct TempString {
    unsigned int flags;
    unsigned int length;
    char* data;
    // Copy the suffix at offset into an owning inline-or-heap string.
    TempString(const char* const& source, volatile char& sourceTag, StringAllocTag& terminator, unsigned int offset) {
        const char* text;
        unsigned int n;
        flags = 0;
        length = 0;
        data = 0;
        text = source + offset;
        n = strlen(text);
        String_ReserveCapacity(this, n);
        terminator.value = sourceTag;
        String_ReplaceRange(this, 0, 0, text, text + n, &terminator);
    }
    // Copy the complete source using the target string allocator-tag ABI.
    TempString(const char* const& source, volatile char& sourceTag, StringAllocTag& terminator) {
        unsigned int n;
        const char* text;
        flags = 0;
        length = 0;
        data = 0;
        text = source;
        n = strlen(text);
        String_ReserveCapacity(this, n);
        terminator.value = sourceTag;
        String_ReplaceRange(this, 0, 0, text, text + n, &terminator);
    }
    // Release heap text only; inline characters live in the string object.
    ~TempString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
};

// Storage for one swing-trail effect object (0x60 bytes); constructed by CTailEffect::CTailEffect from a resource name.
struct TailEffectBox {
    char data[0x60];
    // Construct a swing trail from its resource name.
    TailEffectBox(TempString& s) {
        ((CTailEffect*)this)->__ct(&s);
    }
};
struct PreLoadArray {
    char data[0x2E0];
    // Allocate array storage through the game's array allocator.
    static void* operator new(size_t n) {
        return __nwa__FUl(n);
    }
    // Release array storage if construction exits by exception.
    static void operator delete(void* p) {
        __dla__FPv(p);
    }
    // Construct four animation records and publish their first-element pointer.
    PreLoadArray(void** result) {
        *result = __construct_new_array(this, (void*)GolfPlayerAnimationState_Ctor, (void*)PlayerAnimationState_dtor,
                                        kAnimationStateStride, 4);
    }
};
#pragma opt_common_subs reset
#pragma opt_propagation reset

struct FxString {
    unsigned int flags;
    unsigned int length;
    char* data;
    // Release this retained sound-name string's heap storage, if present.
    ~FxString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
};

#pragma opt_common_subs off
struct SAlloc {
    union {
        volatile char value;
        unsigned int align;
    };
};
// Sound-name ownership path: replacement vector -> swap into player -> old strings
// destroyed backwards. Each FxString owns heap text only when flags bit 31 is set.
// SRef/SGuard below belong to the LENGTH-ERROR exception message, not the sound
// names themselves. Keep their original ABI types; descriptive aliases mark uses.
struct SMarker {
    union {
        volatile char value;
        unsigned int align;
    };
    // Initialize the allocator marker used by sound-name cleanup.
    SMarker() {
        value = 0;
    }
};
struct SRef {
    const void* table;
    int refs, owners;
    char* text;
    // Adopt exception text into the target shared-deleter control block.
    SRef(char* t) {
        refs = 1;
        owners = 1;
        table = s_sharedCharDeleterVtable;
        text = t;
    }
};
typedef SRef SoundNameExceptionTextOwner;
struct SGuard {
    char* text;
    void* allocator;
    // Free exception text unless construction transferred it to SRef.
    ~SGuard() {
        if (text)
            __dla__FPv(text);
    }
};
struct SErrorRoot {
    const void* table;
    char* text;
    SRef* ref;
    // Initialize the target logic-error base vtable.
    SErrorRoot() {
        table = __vt__Q23std11logic_error;
    }
    // Forward destruction to the target standard exception base.
    ~SErrorRoot() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct SLogicError : SErrorRoot {
    // Copy a capacity-error message and give it shared-deleter ownership.
    SLogicError(const char* const& msg, const SAlloc& tmp) {
        SAlloc allocator;
        allocator.value = 0;
        unsigned int n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        text = buffer;
        {
            SGuard guard = {buffer, &allocator};
            ref = new SoundNameExceptionTextOwner(buffer);
            guard.text = 0;
        }
        strcpy(text, msg);
    }
};
struct SError : SLogicError {
    // Finish the length-error type after its message has been constructed.
    SError(const char* msg, const SAlloc& tmp) : SLogicError(msg, tmp) {
        table = __vt__Q23std12length_error;
    }
};
// Throw a vector length error using the original exception-object layout.
inline void SFail(const char* message) {
    SAlloc tmp;
    tmp.value = 0;
    SError error(message, tmp);
    typedef void ThrowFunction(const char*, void*, void*) __attribute__((noreturn));
    ((ThrowFunction*)__throw)(golfPlayerStrings + kTextStdExceptionStdLogicErrorStdLengthError, (void*)&error,
                              (void*)__dt__Q23std12length_errorFv);
}
// Select a capacity-growth bound while retaining reference semantics.
inline const unsigned int& SMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct SStorage {
    FxString* data;
    unsigned int size, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Begin an empty owner for a replacement sound-name vector allocation.
    SStorage(unsigned int* o) : data(0), size(0), capacity(0), owner(o), offset(0) {}
    // Destroy retained strings backwards, then free the still-owned allocation.
    ~SStorage() {
        if (data) {
            unsigned int n = size;
            SMarker marker;
            size -= size;
            FxString* e = data + n;
            for (; n != 0; --n) {
                --e;
                if (e && (e->flags >> 31) != 0)
                    appFreeUni__FPv(e->data);
            }
            appFreeUni__FPv(data);
        }
    }
};
typedef SStorage SoundNameReplacementStorage;
// Transfer the new sound-name storage into the player and retain the old storage for cleanup.
inline void SSwap(char* player, SoundNameReplacementStorage& replacement) {
    unsigned int oldCount = AsCGolfPlayer(player)->loadedSoundNameCount;
    FxString* old = (FxString*)AsCGolfPlayer(player)->loadedSoundNames;
    FxString* replacementData = replacement.data;
    replacement.data = old;
    SMarker marker;
    unsigned int oldCapacity = AsCGolfPlayer(player)->loadedSoundNameCapacity;
    AsCGolfPlayer(player)->loadedSoundNameCapacity = replacement.capacity;
    replacement.capacity = oldCapacity;
    AsCGolfPlayer(player)->loadedSoundNames = replacementData;
    AsCGolfPlayer(player)->loadedSoundNameCount = replacement.size;
    replacement.size = oldCount;
}
#pragma opt_common_subs reset

struct TargetNames {
    const char* names[4];
};
extern const TargetNames s_clubPlacementAnimationNames; // Placement animation names indexed by club category.

// Convert a club-category index into its pointer-table byte offset.
inline void TargetOffset(int slot, unsigned int& result) {
    result = slot * 4;
}

// Compute a floating absolute value with the original conditional evaluation.
inline double TargetAbs(float value) {
    union {
        float f;
        unsigned int u;
    } bits;
    bits.f = value;
    if (bits.u & 0x80000000)
        return -(double)value;
    return value;
}
// Compare using the original double-to-float operand types.
inline bool TargetLess(double a, float b) {
    return a < b;
}
// Truncate a float to an integer frame index.
inline int TargetInt(float value) {
    return (int)value;
}
// Store a truncated integer frame index through the original reference parameter.
inline void TargetIntStore(float value, int& result) {
    result = (int)value;
}
// Convert an integer frame count to float.
inline float TargetFloat(int value) {
    return (float)value;
}

// Return the current golf-document pointer as the legacy byte view.
static inline unsigned char* GetGolfDocBytes(void) {
    return (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
}

// Resolve a negative player index to the current player.
static inline int ResolvePlayerIndex(int index) {
    if (index < 0)
        index = AsCGolfDoc(GetGolfDocBytes())->currentPlayer;
    return index;
}

// Locate a per-player pose record after resolving a negative index.
static inline unsigned char* GetPlayerPoseRecord(CGolfPlayer* self, int index) {
    int offset = ResolvePlayerIndex(index) * kPlayerPoseStride;
    return (unsigned char*)self->playerStates + offset;
}

// Lazily initialize the course/cutscene player-slot command table.
static inline unsigned char* U8013C5CC_Event(void) {
    if (!GetConteModelSlotsSlot()) {
        if (!s_courseSlotTableInitialized) {
            CourseSlotTable_InitEmpty((const char*)&s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        GetConteModelSlotsSlot() = (void*)&s_conteModelSlots;
    }
    return (unsigned char*)GetConteModelSlotsSlot();
}

// Copy a queued command string, preserving its inline/heap representation.
static inline void AssignCommandString(unsigned char* dst, unsigned char* src) {
    if (!(*(unsigned int*)(dst + 8) >> 31) && !(*(unsigned int*)(src + 8) >> 31)) {
        *(unsigned int*)(dst + 8) = *(unsigned int*)(src + 8);
        *(unsigned int*)(dst + 0xC) = *(unsigned int*)(src + 0xC);
        *(unsigned int*)(dst + 0x10) = *(unsigned int*)(src + 0x10);
    } else {
        unsigned int cap;
        unsigned char* first;
        unsigned int len;
        if (*(unsigned int*)(dst + 8) >> 31)
            cap = *(unsigned int*)(dst + 0xC);
        else
            cap = *(unsigned char*)(dst + 8) & 0x7F;
        if (!(*(unsigned int*)(src + 8) >> 31)) {
            first = src + 9;
            len = *(unsigned char*)(src + 8) & 0x7F;
        } else {
            first = *(unsigned char**)(src + 0x10);
            len = *(unsigned int*)(src + 0xC);
        }
        unsigned char alloc;
        String_ReplaceRange(dst + 8, 0, cap, first, first + len, &alloc);
    }
}

// Copy a queued animation command and its owned string.
static inline void AssignAnimationCommand(unsigned char* dst, unsigned char* src) {
    dst[0] = src[0];
    *(int*)(dst + 4) = *(int*)(src + 4);
    AssignCommandString(dst, src);
}

struct U8013C5CC_Vec3 {
    unsigned int x, y, z;
};

struct U8013C5CC_Callable {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3(void*, int, int);
};

struct U8013C5CC_Elem {
    unsigned char flag;
    char pad[3];
    int value;
    unsigned int s0, s1, s2;
};

// Send a player query/update to the Screen actor.
inline void C708Event(void* self, int command, void* argument) {
    IActor_DispatchMessage(self, self, (char*)golfPlayerStrings + kTextScreen, command, (unsigned int)(argument), 0, 0,
                           0);
}
struct C708Virtual {
    virtual void v2();
    virtual void update();
    virtual int done();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void notify(int*);
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void v27();
    virtual void v28();
    virtual void v29();
    virtual void v30();
    virtual void v31();
    virtual void v32();
    virtual void v33();
    virtual void v34();
    virtual void v35();
    virtual void v36();
    virtual void v37();
    virtual void v38();
    virtual void v39();
    virtual void v40();
    virtual void v41();
    virtual void v42();
    virtual void v43();
    virtual void v44();
    virtual void v45();
    virtual void v46();
    virtual void v47();
    virtual void v48();
    virtual void v49();
    virtual void v50();
    virtual void v51();
    virtual void refresh();
};

#undef ENTRY
#undef OBJ

struct ResourceString {
    unsigned int storageHeader;
    unsigned int lengthOrInlineBytes;
    char* data;
    // Initialize an empty inline string with no heap allocation.
    ResourceString() : storageHeader(0), lengthOrInlineBytes(0), data(0) {}
    ~ResourceString();
};

// Release heap text when the string's representation tag requires it.
inline ResourceString::~ResourceString() {
    if (storageHeader >> 31)
        appFreeUni__FPv(data);
}

#pragma does_not_return __throw

struct IntVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};

// Helpers are declared above with their unit signatures.

struct StringRep {
    char* vtable;
    unsigned int refs;
    unsigned int size;
    unsigned int capacity;
    char* data;
};
struct ThrowString {
    char* data;
    char* exception;
};

unsigned int Vector_GetCount(void*); // Read the vector size.

struct ResourceInfo {
    char pad0[0x18];
    char transform[0x10];
    char* text;
};
struct ResourceEntry {
    void* node;
    ResourceInfo* info;
};
struct ResourceTable {
    char pad0[0x13C];
    ResourceEntry* entries;
    unsigned int count;
};
struct ResourceParams {
    char pad0[8];
    void* transform;
    char padC[0xC];
    void* extra;
    char pad1C[0xC];
};

// Construct and throw a vector length error with its owned message string.
static void throwLengthError(char* local, char* marker) {
    char* copy = (char*)__nwa__FUl((unsigned int)(strlen(golfPlayerStrings + kTextVectorLengthError) + 1));
    StringRep* rep = (StringRep*)__nw__FUl(0x10);
    if (rep) {
        rep->refs = 1;
        rep->size = 1;
        rep->vtable = s_sharedCharDeleterVtable;
        rep->data = copy;
    }
    __dla__FPv(rep);
    strcpy(copy, golfPlayerStrings + kTextVectorLengthError);
    ThrowString t = {__vt__Q23std12length_error, 0};
    __throw(golfPlayerStrings + kTextStdExceptionStdLogicErrorStdLengthError, &t, __dt__Q23std12length_errorFv);
}

struct TargetRep {
    char* vtable;
    unsigned int refs, size;
    char* data;
    // Adopt exception text into its shared-deleter control block.
    TargetRep(char* p) {
        refs = 1;
        size = 1;
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
struct TargetGuard_u8013AE84 {
    char* data;
    char* allocator;
    // Hold newly allocated error text until control-block construction succeeds.
    TargetGuard_u8013AE84(char* p, char* a) : data(p), allocator(a) {}
    // Free error text that was not transferred to its control block.
    ~TargetGuard_u8013AE84() {
        if (data)
            __dla__FPv(data);
    }
};

struct TargetErrorRoot {
    char* vtable;
    char* data;
    TargetRep* rep;
    // Initialize the target logic-error base vtable.
    TargetErrorRoot() {
        vtable = __vt__Q23std11logic_error;
    }
    // Forward cleanup to the target standard exception base destructor.
    ~TargetErrorRoot() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct TargetErrorBase_u8013AE84 : TargetErrorRoot {
    // Copy the error message and construct its shared-deleter owner.
    TargetErrorBase_u8013AE84(const char* const& text, char* allocator) {
        unsigned int n = strlen(text) + 1;
        data = (char*)__nwa__FUl(n * 2 - n);
        {
            char* copy = data;
            TargetGuard_u8013AE84 guard(copy, allocator);
            rep = new TargetRep(copy);
            guard.data = 0;
        }
        strcpy(data, text);
    }
};
struct TargetError_u8013AE84 : TargetErrorBase_u8013AE84 {
    // Complete the vector length-error type after constructing its message.
    TargetError_u8013AE84(const char* text, char* allocator) : TargetErrorBase_u8013AE84(text, allocator) {
        vtable = __vt__Q23std12length_error;
    }
};
struct TargetCharAllocator {
    union {
        char value;
        unsigned int align;
    };
    // Initialize the one-byte allocator tag passed through the exception ABI.
    TargetCharAllocator() : value(0) {}
};
// Throw the integer-vector capacity error using the original allocator/exception layout.
inline void targetThrow(const char* text) {
    TargetCharAllocator a;
    TargetError_u8013AE84 error(text, &a.value);
    __throw(golfPlayerStrings + kTextStdExceptionStdLogicErrorStdLengthError, &error, __dt__Q23std12length_errorFv);
}
struct TargetStorage {
    unsigned int* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int start;
    // Begin an empty owner for replacement attached-effect ID storage.
    TargetStorage(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), start(0) {}
    // Free the allocation left after swapping with the live ID vector.
    ~TargetStorage() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Select the larger capacity-growth bound by reference.
inline const unsigned int& targetMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Throw the sequence-vector capacity error using the original exception layout.
inline void targetThrow868(const char* text) {
    TargetCharAllocator a;
    const TargetError_u8013AE84& error = TargetError_u8013AE84(text, &a.value);
    typedef void ThrowFunction(const char*, void*, void*) __attribute__((noreturn));
    ((ThrowFunction*)__throw)(golfPlayerStrings + kTextStdExceptionStdLogicErrorStdLengthError, (void*)&error,
                              __dt__Q23std12length_errorFv);
}
// Select the larger growth bound while retaining volatile reads.
inline const volatile unsigned int& targetMax868(const unsigned int& a, const volatile unsigned int& b) {
    return a < b ? b : a;
}
// Calculate the sequence-ID vector growth capacity within its maximum element count.
inline unsigned int targetCapacity868(IntVector* self, const volatile unsigned int& needed) {
    unsigned int half;
    unsigned int threeFifths;
    unsigned int capacity = self->capacity;
    if (needed > 0x3FFFFFFF - capacity)
        targetThrow868(golfPlayerStrings + kTextVectorLengthError);
    if (capacity < 0x15555555) {
        threeFifths = (capacity + 1) * 3 / 5;
        capacity += targetMax868(threeFifths, needed);
    } else if (capacity < 0x2AAAAAAA) {
        half = (capacity + 1) / 2;
        capacity += targetMax868(half, needed);
    } else
        capacity = 0x3FFFFFFF;
    if (capacity > 0x3FFFFFFF)
        targetThrow868(golfPlayerStrings + kTextVectorLengthError);
    return capacity;
}
struct TargetVector868 {
    void** data;
    unsigned int size, capacity;
};

struct TargetAllocator {
    union {
        char value;
        unsigned int alignment;
    };
    // Leave the allocator tag unspecified, as in the original empty constructor.
    TargetAllocator() {}
    // Copy the allocator tag when copying target-library container state.
    TargetAllocator(const TargetAllocator& other) : value(other.value) {}
};

typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;
typedef float f32;
struct VectorComponents {
    float v[3];
};
struct HoleTable {
    s32 holes[12];
};
struct RuntimeObject {
    virtual char* getType();
};
struct PlayerDispatch {
    virtual void slot00();
    virtual void slot04();
    virtual void slot08();
    virtual void slot0C();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1C();
    virtual void slot20();
    virtual void slot24();
    virtual void fn30();
    virtual void slot34();
    virtual void slot38();
    virtual void slot3C();
    virtual void slot40();
    virtual void slot44();
    virtual void slot48();
    virtual void slot4C();
    virtual void slot50();
    virtual void slot54();
    virtual void slot58();
    virtual void slot5C();
    virtual void slot60();
    virtual void slot64();
    virtual void slot68();
    virtual void slot6C();
    virtual void slot70();
    virtual void slot74();
    virtual void slot78();
    virtual void slot7C();
    virtual void slot80();
    virtual void slot84();
    virtual void slot88();
    virtual void slot8C();
    virtual void slot90();
    virtual void slot94();
    virtual void slot98();
    virtual void slot9C();
    virtual void slotA0();
    virtual void slotA4();
    virtual void slotA8();
    virtual void slotAC();
    virtual void slotB0();
    virtual void slotB4();
    virtual void slotB8();
    virtual void slotBC();
    virtual void slotC0();
    virtual void slotC4();
    virtual void slotC8();
    virtual void slotCC();
    virtual void fnD0();
};

// Message packet ABI; the sender chooses whether each argument is a value or a pointer.
struct Cmd {
    u32 unk0;
    s32 message;
    void* argument; // Payload or output pointer; interpretation depends on message.
    void* argument2;
    u32 argument3;
    u32 argument4;
    u32 argument5;
    u32 scheduledTimeMs;
};

// Message payload guide (a1=argument, a2=argument2; value words are cast from
// void*, pointers refer to caller-owned storage). Messages are synchronous here;
// a future scheduledTimeMs invokes fn30 and returns without consuming payloads.
// Queries: 3 -> float* a1 heading radians; 4 -> Vec3* a1 placement offset;
// 5 -> u32* a1 contact flag + float* a2; 7 -> void** a1 bone, const char* a2 name;
// 9 -> float* a1 clamped slope radians; 21 -> u32* a1 animation mode;
// 30 -> value a1 slot, affine-matrix* a2; 32 -> float* a1 elapsed *shot seconds
// (left unchanged if the swing name is absent); 35 -> Vec3* a1 position;
// 39/49 -> u32* a1 completion; 41 -> int* a1, value a2 slot; 52 -> u8* a1.
// Pose/animation: 2 adds float* a1 radians; 10007 sets float* a1 radians;
// 10008 sets Vec3* a1 world position; 10005 selects const char* a1 and uses
// INTEGER value a2 converted to float time offset; 10006 queues const char* a1
// (null requests the default ready name). 11 tests Vec3* a1 at eight headings;
// 25 takes power-mode VALUE a1. 36/37 release resources for VALUE a1 slot;
// 38 begins avatar load for VALUE a1 slot, value a2==1 selects load kind 4;
// 40 applies it. 46/47 enter/leave scripted slots; 48 forwards a1 to load-list setup.
// Preview placement 27/28 reads Vec3* a1 and const char* a2; message 28 also
// derives club visibility from a1's nullness. Callers must satisfy both uses.
// Winner 26 takes animation names a1/a2 and a world Vec3* in argument3.
// Resource pointers returned by queries remain owned by their model/resource.

// Width views for external records whose shared layout is still incomplete.
// Macros are needed here: inline functions emit extra calls in HandleMsg's nested helpers.
#define pointerFieldAt(base, offset) (*(char**)((char*)(base) + (offset)))
#define unsignedFieldAt(base, offset) (*(u32*)((char*)(base) + (offset)))
#define intFieldAt(base, offset) (*(s32*)((char*)(base) + (offset)))
#define byteFieldAt(base, offset) (*(u8*)((char*)(base) + (offset)))
#define floatFieldAt(base, offset) (*(f32*)((char*)(base) + (offset)))
#define vectorFieldAt(base, offset) (*(VectorComponents*)((char*)(base) + (offset)))
#define playerModelBytes(player, index) ((*(char**)&AsCGolfPlayer(player)->playerModels) + (index) * kPlayerModelStride)
#define playerAnimationBytes(player, index) \
    ((*(char**)&AsCGolfPlayer(player)->animationStates) + (index) * kAnimationStateStride)
#define playerPoseBytes(player, index) ((*(char**)&AsCGolfPlayer(player)->playerStates) + (index) * kPlayerPoseStride)
extern "C++" {
#pragma push
#pragma always_inline on
// Send a player-originated message; preserve each caller's payload conversion and evaluation count.
template <class Argument>
static inline void sendPlayerEvent(void* player, int textOffset, int message, Argument argument) {
    IActor_DispatchMessage(player, player, golfPlayerStrings + textOffset, message, (u32)argument, 0, 0, 0);
}
#pragma pop
}

// Keep these identical in the dispatcher and pose resolver for MWCC range propagation.
#define currentGolfDocBytes() (*(char**)&AsCSharedDoc((u32)g_sharedDoc)->golfDoc)
#define currentPlayerIndex() (*(u8*)(currentGolfDocBytes() + 0x3C1))
// Locate the pose record, using the current player for a negative index.
static inline char* rotationState(CGolfPlayer* self, s32 index) {
    if (index < 0)
        index = currentPlayerIndex();
    return playerPoseBytes(self, index);
}
// Clear the numeric sign by conditional negation while preserving the original float operations.
static inline f32 absoluteValue(f32 value) {
    union {
        f32 f;
        u32 u;
    } bits;
    bits.f = value;
    if (bits.u & 0x80000000)
        value = -value;
    return value;
}
// Test the animation-time tolerance.
static inline bool negligible(f32 value) {
    return value < kGolfPlayerNearZeroTolerance;
}
// Set the indexed PLAYER scene entry visible or hidden.
static inline void setPlayerUI(u32 manager, s32 index, s32 visible) {
    const char* name = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, index);
    DrawScene_SetNamedObjectEnabled((void*)(manager), name, visible);
}
// Bind a model to the indexed PLAYER scene entry.
static inline void bindPlayerUI(u32 manager, s32 index, u32 object) {
    const char* name = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, index);
    DrawScene_SetNamedObjectPuppet((void*)(manager), name, (void*)(object));
}
// Bind the player-model owner's primary puppet to its named shadow/effect group.
static inline void bindPetEffect(u32 manager, const char* name, void* pet) {
    void* model = PlayerModel_GetPuppet(pet);
    ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv((void*)(manager), name, model);
}

// Walk the active task's type chain to find the golf task type.
static inline bool hasActivePlayerType() {
    void* obj = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_lastGolfTaskTypeProbeTarget = obj;
    if (obj) {
        char* node = ((RuntimeObject*)obj)->getType();
        while (node) {
            if (node == (char*)&s_golfTaskTypeInfo)
                return true;
            node = pointerFieldAt(node, 4);
        }
        return false;
    } else {
        return false;
    }
}
// Lazily initialize the render defaults used to create shadow entries.
static inline void* effectSettings(void* registration) {
    if (!Sing_805DC7B0()) {
        if (!s_renderPacketInitialized) {
            RenderPacket_Init(s_renderPacket);
            __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, registration);
            s_renderPacketInitialized = 1;
        }
        Sing_805DC7B0() = s_renderPacket;
    }
    return Sing_805DC7B0();
}
// Refresh player lighting from the scene and the terrain beneath the head bone.
static inline void refreshPlayerLighting(CGolfPlayer* self, s32 index) {
    *(WVector*)((char*)&AsCGolfPlayer(self)->lightDirection) = *(WVector*)(s_sceneLighting + 4);
    (*(u32*)&AsCGolfPlayer(self)->ambientLightColor) =
        Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x14), kFloat_1_GolfPlayer);
    if (index < 0)
        index = currentPlayerIndex();
    if (index >= 0 && index < 4) {
        void* part =
            WPuppet_FindBoneByName(PlayerModel_GetPuppet(playerModelBytes(self, index)), golfPlayerStrings + kTextHead);
        if (part) {
            float gain = ((CQuadTree*)CQuadTree_GetInstance())
                             ->DefaultLightingFactor(&((PuppetBoneView*)part)->worldMatrix.position);
            (*(u32*)&AsCGolfPlayer(self)->lightColor) = Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x10), gain);
        }
    }
}
// Build a swing animation name from club, power mode, impact timing, and short-putt direction.
static inline void selectName(CGolfPlayer* self, s32 full) {
    if ((((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == kClubWood ||
         ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == kClubIron) &&
        AsCGolfDoc(currentGolfDocBytes())->impactGrade < 2) {
        char prefix[32] = {0};
        char suffix[32] = {0};
        if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 2 &&
            ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == 0)
            strcpy(prefix, golfPlayerStrings + kTextDoublePowerPrefix);
        else if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != 0)
            strcpy(prefix, golfPlayerStrings + kTextPowerPrefix);
        if (AsCGolfDoc(currentGolfDocBytes())->shotImpactGaugeX > AsCGolfDoc(currentGolfDocBytes())->pangyaTargetX)
            strcpy(suffix, golfPlayerStrings + kTextEarlyPrefix);
        else
            strcpy(suffix, golfPlayerStrings + kTextLatePrefix);
        strcpy((char*)&AsCGolfPlayer(self)->swingAnimationName,
               String_FormatTemporary(golfPlayerStrings + kTextSwingNameFormat,
                                      (char*)&AsCGolfPlayer(self)->shotAnimationPrefix[0], prefix, suffix));
    } else if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != 0) {
        if (!full || ((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 1 ||
            ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category != 0)
            strcpy((char*)&AsCGolfPlayer(self)->swingAnimationName,
                   String_FormatTemporary(golfPlayerStrings + kTextPowerSwingFormat,
                                          (char*)&AsCGolfPlayer(self)->shotAnimationPrefix[0]));
        else
            strcpy((char*)&AsCGolfPlayer(self)->swingAnimationName,
                   String_FormatTemporary(golfPlayerStrings + kTextDoublePowerSwingFormat,
                                          (char*)&AsCGolfPlayer(self)->shotAnimationPrefix[0]));
    } else {
        if (((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == 3 &&
            AsCGolfPlayer(self)->distanceToHole < kShortPuttDistance_GolfPlayer) {
            char* mgr = currentGolfDocBytes();
            u32 pos[3];
            f32 delta[3];
            char* hole = (*(char**)&AsCGolfDoc(mgr)->holeRecords) + AsCGolfDoc(mgr)->currentHoleIndex * 32;
            pos[0] = unsignedFieldAt(hole, 0xc);
            pos[1] = unsignedFieldAt(hole, 0x10);
            pos[2] = unsignedFieldAt(hole, 0x14);
            __mi__FRC7WVectorRC7WVector(delta, pos, GolfBall__Fv());
            f32 angle = Vector_SignedAngleXZ(delta, playerAnimationBytes(self, byteFieldAt(mgr, 0x3c1)) + 0x20);
            f32 absolute;
            if (angle > kFloat_0_GolfPlayer)
                absolute = angle;
            else
                absolute = -angle;
            if (absolute < kShortPuttHeadingTolerance_GolfPlayer) {
                strcpy((char*)&AsCGolfPlayer(self)->swingAnimationName, golfPlayerStrings + kTextShortPutt);
                return;
            }
        }
        strcpy((char*)&AsCGolfPlayer(self)->swingAnimationName, (char*)&AsCGolfPlayer(self)->shotAnimationPrefix[0]);
    }
}
// Test the course/character/score/hole combination for the special victory animation.
static inline bool specialHole() {
    bool result = false;
    s32 index = AsCSharedDoc(g_sharedDoc)->golfRuleType == 1
                    ? ((CGolfDoc*)currentGolfDocBytes())->ResolveMatchTurnFromHoleStrokes()
                    : currentPlayerIndex();
    s32 hole = AsCGolfDoc(currentGolfDocBytes())->currentHoleIndex;
    s32 strokes = byteFieldAt(GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), (u8)index), 0x28 + hole);
    s32 par = byteFieldAt((*(char**)&AsCGolfDoc(currentGolfDocBytes())->holeRecords) +
                              AsCGolfDoc(currentGolfDocBytes())->currentHoleIndex * 32,
                          0x18);
    s32 difference = strokes - par;
    s32 kind = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                unsignedFieldAt(((u32)g_sharedDoc), index * 0x1d4 + 0x120));
    HoleTable table = *(HoleTable*)s_specialVictoryHoleIndices;
    if (SharedDoc_GetCourseId() == 12 && difference == -2 && kind == 7) {
        volatile s32* entries = table.holes;
        char* manager = currentGolfDocBytes();
        for (s32 i = 0; i < 12; ++i) {
            s32 entry = entries[i];
            u8 hole = *(volatile u8*)(manager + 0x3b6);
            if (entry == hole)
                result = true;
        }
    }
    return result;
}

extern char* strcpy(char*, const char*);

typedef Vec3f Vec3;
struct Transform {
    unsigned char data[0x18];
};

extern unsigned char GolfDoc_GetPlayerCount();
extern void* GolfDoc_GetCurrent();
extern void* GolfPlayer_GetCurrentEntryByIndex(unsigned char);

extern void* SharedDoc_GetItemInventory();

typedef Vec3f V3;
struct I3 {
    int x, y, z;
};
struct Mat {
    float v[12];
};
struct Bounds {
    V3 min, max;
};

#define PIDX \
    (playerIndex < 0 ? (int)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer : playerIndex)

struct U80146020_Manager {
    char pad[0xB90];
    void* current;
};

struct U80146020_Entry {
    char pad[0x2B9];
    unsigned char count;
};
struct U80146020_Panel {
    char pad[0x44];
    void* field44;
    char pad2[0xD8];
    unsigned char field120[0x70];
};
struct U80146020_ListItem {
    char pad[0xB4];
};
struct U80146020_Obj {
    virtual ~U80146020_Obj();
};

struct LimitObj {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void v27();
    virtual void v28();
    virtual void v29();
    virtual void v30();
    virtual void v31();
    virtual void v32();
    virtual void v33();
    virtual void v34();
    virtual void v35();
    virtual void v36();
    virtual void v37();
    virtual void v38();
    virtual void v39();
    virtual void v40();
    virtual void v41();
    virtual void v42();
    virtual void v43();
    virtual void v44();
    virtual void v45();
    virtual int getLimit(float*, void*);
};

struct UnitVec {
    unsigned int x, y, z;
};
struct UnitObject {
    virtual void slot08();
    virtual void slot0C();
    virtual void slot10();
    virtual void update(void*, int, int);
};

// Cache animation-derived grip offsets and swing reaches for one player or all players.
void CGolfPlayer::CacheSwingPlacements(int selectedPlayerIndex) {
    InstanceTable_ClearCachedResources(InstanceTable_Get());
    ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))
        ->GetItemSelection(AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
    void* defaultClubSetId = ItemInventory_GetClubSetIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, 0);
    unsigned int defaultClubModels = (int)ClubModel_CreateForItem(InstanceTable_Get(), (unsigned int)defaultClubSetId);
    unsigned int objectOffset = 0;
    SwingPlacement* clubPlacements = &byPlayerAndClub[0][0];
    SwingPlacement* defaultPlacement = defaultByPlayer;
    int index = 0;
    int recordOffset = 0;
    for (; index < playerIndexOffset + AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount;
         ++index, recordOffset += 0x1D4, objectOffset += kPlayerModelStride, ++clubPlacements, ++defaultPlacement) {
        if (selectedPlayerIndex != -1 && index != selectedPlayerIndex)
            continue;
        if (playerIndexOffset == 0) {
            void* obj = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                   static_cast<unsigned char>(index));
            if (*reinterpret_cast<int*>(static_cast<char*>(obj) + 8) == 3)
                continue;
        }
        void* characterSelection = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))
                                       ->GetItemSelection(static_cast<unsigned char>(index));
        void* characterData = ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                              *reinterpret_cast<void**>(characterSelection));
        void* manager = InstanceTable_Get();
        void* model = ClubModel_CreateForItem(
            manager,
            (unsigned int)*reinterpret_cast<void**>(reinterpret_cast<char*>(g_sharedDoc) + recordOffset + 0x1CC));
        void* animationObjects =
            PlayerModel_GetAnimationResourceVector(static_cast<char*>(playerModels) + objectOffset);
        unsigned int* animationCursor = *reinterpret_cast<unsigned int**>(animationObjects);
        while (animationCursor != *reinterpret_cast<unsigned int**>(animationObjects) +
                                      *reinterpret_cast<unsigned int*>(static_cast<char*>(animationObjects) + 4)) {
            void* object = reinterpret_cast<void*>(*animationCursor);
            SwingPlacement* dest = clubPlacements;
            for (int clubCategory = 0; clubCategory < 4; ++clubCategory, ++dest)
                ReadClubSwingPlacement(object, model, characterData, clubCategory, dest);
            ReadClubSwingPlacement(object, reinterpret_cast<void*>(defaultClubModels), characterData, 0,
                                   defaultPlacement);
            animationCursor++;
        }
    }
}

// Read ball-distance/grip keys and scale one club category's placement to the character.
void CGolfPlayer::ReadClubSwingPlacement(void* animationPuppet, void* clubModels, void* characterData, int clubCategory,
                                         void* placementOutput) {
    // category 0..3 selects wood/iron/wedge/putter; output holds four floats:
    // [placement x,y,z, attachment-axis offset]. Starts at zero on missing keys.
    // Range time is seconds -> rounded frame using the animation FPS constant.
    // The club/character branch tests OR but dereferences both dependencies;
    // callers must provide both when that branch can run.
    unsigned int clubPointerOffset = 0;
    TargetNames names = s_clubPlacementAnimationNames;
    float* placement = (float*)placementOutput;
    Vec3_Zero(placement);
    TargetOffset(clubCategory, clubPointerOffset);
    placement[3] = kFloat_0_GolfPlayer;
    float* animationRange =
        WPuppet_FindAnimationRangeByName(animationPuppet, *(const char**)((char*)&names + clubPointerOffset));
    if (animationRange) {
        float time = animationRange[0];
        float rate;
        int index = -1;
        int other;
        rate = kAnimationFramesPerSecond_GolfPlayer;
        int frame = (int)(rate * time + kFloat_0_5_GolfPlayer);
        WPuppet_FindKeyRange(animationPuppet, frame, frame, &index, &other);
        if (index > -1) {
            const char* keyText = *(const char**)((char*)((WPuppet*)animationPuppet)->keyRecords + index * 12 + 8);
            String_ScanTokensAfterSubstring(&keyText, golfPlayerStrings, golfPlayerStrings + kTextF, placement);
            String_ScanTokensAfterSubstring(&keyText, golfPlayerStrings + kTextGripPos, golfPlayerStrings + kTextF,
                                            placement + 3);
            float depth;
            float zero;
            zero = kFloat_0_GolfPlayer;
            depth = kSwingPlacementDepth_GolfPlayer;
            placement[1] = zero;
            placement[2] = depth;
            if (clubModels || characterData) {
                clubCategory = (int)((char*)clubModels + clubPointerOffset);
                char* entry = (char*)clubCategory;
                void** a = (void**)((void* (*)(void*, const char*))WPuppet_FindNamedBoneEntry)(
                    *(void**)(entry + 4), golfPlayerStrings + kTextBall);
                void** b = (void**)((void* (*)(void*, const char*))WPuppet_FindNamedBoneEntry)(
                    *(void**)(entry + 4), golfPlayerStrings + kTextBallTarget);
                if (a && b) {
                    char matrix[0x30];
                    Matrix_SetIdentityAffine(matrix);
                    Model_SetMatrixRecursive(*(void**)(entry + 4), 0, matrix);
                    WPuppet_UpdateBoneWorldMatrices(*(void**)(entry + 4), 0, 0, kFloat_1_GolfPlayer);
                    void* origin = &((PuppetBoneView*)a[0])->worldMatrix;
                    // Transform Ball/Ball_Target resource attachments through
                    // their bone matrices, then scale the separation by character.
                    TargetVec v4, v3, v2, v1;
                    Matrix_TransformPoint(&v1, &((BoneAttachmentData*)b[1])->position,
                                          &((PuppetBoneView*)b[0])->worldMatrix);
                    Matrix_TransformPoint(&v2, &((BoneAttachmentData*)a[1])->position, origin);
                    __mi__FRC7WVectorRC7WVector(&v3, &v2, &v1);
                    __ml__FRC7WVectorf(&v4, &v3, ((IFF_STRUCT::sChar*)characterData)->scale);
                    placement[0] += v4.x;
                    placement[2] -= v4.z;
                }
            }
        }
    }
}

// Forward the named-bone lookup without changing the entry pointer.
void* WPuppet_FindNamedBoneEntryForward(void* object, const char* name) {
    return WPuppet_FindNamedBoneEntry(object, name);
}

// Return a bone's affine world-matrix address.
void* PuppetBone_GetWorldMatrix(void* self) {
    return static_cast<char*>(self) + 0x78;
}

// Register four swing trails and allocate the four per-player animation records.
void CGolfPlayer::OnPreLoadInit() {
    char* const& names = (char*)golfPlayerStrings;
    StringAllocTag tag1, term1, tag2, term2, tag3, term3, tag4, term4;
    float one = kFloat_1_GolfPlayer;
    float zero = kFloat_0_GolfPlayer;
    lightType = 2;
    ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
        g_sceneManager, 4, 0, 0, names + kTextClubActor, 0, 8, 0, one, zero);
    NamedEffectList_SetDistance(g_sceneManager, golfPlayerStrings + kTextClubActor, kPlayerSceneDistance640);
    // Phase 1: build four swing-trail effects from the ~swinglight*.gsr resource names (default, rainbow, pink,
    // blue) and store them in tailEffects[0..3]. Each is then registered as a preload request (kind code 6) under its
    // "tail*" scene name, and that scene object is initially disabled. tag1..4/term1..4 are the volatile
    // allocator-tag bytes TempString needs; their values are never read.
    {
        TempString s(names, tag1.value, term1, 0x30);
        tailEffects[0] = new TailEffectBox(s);
    }
    {
        TempString s(golfPlayerStrings + kTextSwinglight02Gsr, tag2.value, term2);
        tailEffects[1] = new TailEffectBox(s);
    }
    {
        TempString s(golfPlayerStrings + kTextSwinglight01PinkGsr, tag3.value, term3);
        tailEffects[2] = new TailEffectBox(s);
    }
    {
        TempString s(golfPlayerStrings + kTextSwinglight01BlueGsr, tag4.value, term4);
        tailEffects[3] = new TailEffectBox(s);
    }
    ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
        g_sceneManager, 6, 0, tailEffects[0], golfPlayerStrings + kTextTail, 0, 0, 0, kFloat_1_GolfPlayer,
        kFloat_0_GolfPlayer);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kTextTail, 0);
    ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
        g_sceneManager, 6, 0, tailEffects[1], golfPlayerStrings + kTextTailRainbow, 0, 0, 0, kFloat_1_GolfPlayer,
        kFloat_0_GolfPlayer);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kTextTailRainbow, 0);
    ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
        g_sceneManager, 6, 0, tailEffects[2], golfPlayerStrings + kTextTailPink, 0, 0, 0, kFloat_1_GolfPlayer,
        kFloat_0_GolfPlayer);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kTextTailPink, 0);
    ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
        g_sceneManager, 6, 0, tailEffects[3], golfPlayerStrings + kTextTailBlue, 0, 0, 0, kFloat_1_GolfPlayer,
        kFloat_0_GolfPlayer);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, golfPlayerStrings + kTextTailBlue, 0);
    void* array = 0;
    new PreLoadArray(&array);
    animationStates = array;
    for (unsigned char i = 0; i < 4; ++i) {
        void* manager = g_sceneManager;
        void* name1 = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, (unsigned char)i);
        ((void (*)(void*, int, int, void*, const char*, int, int, int, float, float))ResourcePreloadQueue_AddRequest)(
            manager, 4, 0, 0, (const char*)name1,
            (int)(static_cast<char*>(animationStates) + i * kAnimationStateStride + 8), 8, 0, kFloat_1_GolfPlayer,
            kFloat_0_GolfPlayer);
        manager = g_sceneManager;
        void* name2 = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, (unsigned char)i);
        NamedEffectList_SetDistance(manager, (const char*)name2, kPlayerSceneDistance640);
    }
}

// Initialize animation timing, visibility, interpolation, and the model matrix.
void* GolfPlayerAnimationState_Ctor(void* self) {
    float one = kFloat_1_GolfPlayer;
    float zero = kFloat_0_GolfPlayer;
    char* p = static_cast<char*>(self);
    *reinterpret_cast<float*>(p) = zero;
    *reinterpret_cast<unsigned int*>(p + 4) = 0;
    p[0x38] = 0;
    *reinterpret_cast<unsigned int*>(p + 0x5C) = 0;
    *reinterpret_cast<unsigned int*>(p + 0x60) = 0;
    *reinterpret_cast<unsigned int*>(p + 0xA4) = 0;
    *reinterpret_cast<unsigned int*>(p + 0xA8) = 0;
    *reinterpret_cast<float*>(p + 0xAC) = one;
    *reinterpret_cast<float*>(p + 0xB0) = zero;
    Matrix_SetIdentityAffine(p + 8);
    p[0x39] = 0;
    memset(p + 0x64, 0, 0x40);
    return self;
}

#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma opt_propagation off
// Load player models and sound banks, bind scene entries, and cache swing key data.
void CGolfPlayer::OnLoad() {
    // Phase 1: release prior model owners and construct four empty replacements.
    register const char* initialAnimationName;
    register unsigned char playerIndex;
    register char* playerBytes;
    register int avatarLoadKind;
    register int visiblePlayerCount;
    register char* characterSelection;
    register unsigned int playerModelOffset;
    register int storyLoadMode;
    register unsigned int sharedPlayerOffset;
    playerBytes = reinterpret_cast<char*>(this);
    if (g_headIcon)
        ((CHeadIcon*)g_headIcon)->ReleasePlayerObjects();
    void* oldPlayerModels = AsCGolfPlayer(playerBytes)->playerModels;
    if (oldPlayerModels) {
        __destroy_new_array(oldPlayerModels, (void*)PlayerModel_Dtor);
        AsCGolfPlayer(playerBytes)->playerModels = 0;
    }
    void* newPlayerModels = __nwa__FUl(0xA90);
    if (newPlayerModels)
        newPlayerModels = __construct_new_array(newPlayerModels, (void*)PlayerModel_ClearState, (void*)PlayerModel_Dtor,
                                                kPlayerModelStride, 4);
    AsCGolfPlayer(playerBytes)->playerModels = newPlayerModels;
    char* mgr = reinterpret_cast<char*>(g_sharedDoc);
    unsigned char count = AsCGolfDoc(*reinterpret_cast<char**>(mgr + 0xB90))->playerCount;
    visiblePlayerCount = 2;
    if (count <= 2)
        visiblePlayerCount = count;
    int mode = 0;
    if (AsCSharedDoc(mgr)->gameMode == 6)
        ((int (*)(void*, void*, const char*, int, int*, int, int, int))IActor_DispatchMessage)(
            this, this, golfPlayerStrings + kTextStoryConte, 4, &mode, 0, 0, 0);
    initialAnimationName = golfPlayerStrings + kTextEntranceAnimation;
    if (mode != 0)
        initialAnimationName = golfPlayerStrings + kTextMTSTORYSTAND;
    // Phase 2: load visible characters with the fallback item selection; bind
    // their puppets/shadows, then retain each player's acquired sound-bank name.
    for (playerIndex = 0; playerIndex < kPlayerSlotCount; ++playerIndex) {
        characterSelection = static_cast<char*>(
            ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->GetItemSelection(playerIndex));
        if ((int)playerIndex < visiblePlayerCount) {
            char* playerManager = reinterpret_cast<char*>(g_sharedDoc);
            avatarLoadKind = 2;
            if (*reinterpret_cast<int*>(playerManager + playerIndex * 0x1D4 + 0x1DC) != 0)
                avatarLoadKind = 1;
            storyLoadMode = mode;
            if (*reinterpret_cast<int*>(static_cast<char*>(GolfPlayer_GetEntryByIndex(
                                            (*(void**)&AsCSharedDoc(playerManager)->golfDoc), playerIndex)) +
                                        8) != 3) {
                int one = 1;
                sharedPlayerOffset = playerIndex * 0x1D4;
                playerModelOffset = playerIndex * kPlayerModelStride;
                if (AvatarModel_LoadCharacterResources(
                        *reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->playerModels) + playerModelOffset,
                        characterSelection, reinterpret_cast<char*>(g_sharedDoc) + sharedPlayerOffset + 0x190, one,
                        avatarLoadKind, one, playerIndex, storyLoadMode) == 0) {
                    ItemSelection_RefreshPrimaryItems(characterSelection);
                    AvatarModel_LoadCharacterResources(
                        *reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->playerModels) + playerModelOffset,
                        characterSelection, reinterpret_cast<char*>(g_sharedDoc) + sharedPlayerOffset + 0x190, 1,
                        avatarLoadKind, 0, playerIndex, storyLoadMode);
                }
                UpdatePuppetBoundsForAnimationList(
                    *reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->playerModels) + playerModelOffset);
                *reinterpret_cast<int*>(*reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->animationStates) +
                                        playerIndex * kAnimationStateStride + 0x60) = 0;
                *reinterpret_cast<char*>(*reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->animationStates) +
                                         playerIndex * kAnimationStateStride + 0x39) = 0;
                void* property = g_sceneManager;
                void* seq = PlayerModel_GetPuppet(*reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->playerModels) +
                                                  playerModelOffset);
                const char* name = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, (unsigned char)playerIndex);
                DrawScene_SetNamedObjectPuppet(property, name, seq);
                if (!Sing_805DC7B0()) {
                    if (!s_renderPacketInitialized) {
                        RenderPacket_Init((void*)s_renderPacket);
                        __register_global_object((void*)s_renderPacket, (void*)RenderPacket_Destroy,
                                                 (void*)s_renderPacketDtorRegistration_golfPlayer);
                        s_renderPacketInitialized = 1;
                    }
                    Sing_805DC7B0() = (void*)s_renderPacket;
                }
                GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
                    g_shadowManager, golfPlayerStrings + kTextChar, kFloat_0_05_PlayerShadow, 0x10,
                    golfPlayerStrings + kTextSun, static_cast<char*>(Sing_805DC7B0()) + 0x4C);
                void* actors = g_shadowManager;
                ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
                    actors, golfPlayerStrings + kTextChar,
                    PlayerModel_GetPuppet(*reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->playerModels) +
                                          playerModelOffset));
            }
            CGolfPlayer_SetPlayerAnimation(this, initialAnimationName, kFloat_0_GolfPlayer, (int)(playerIndex));
        }
        if (playerIndex <
            AsCGolfDoc(*reinterpret_cast<char**>(reinterpret_cast<char*>(g_sharedDoc) + 0xB90))->playerCount) {
            if (characterSelection) {
                unsigned int stringIndex = *reinterpret_cast<unsigned int*>(characterSelection) & 0x03FFFFFFu;
                FxString info;
                info.flags = 0;
                info.length = 0;
                info.data = 0;
                if (SoundName_BuildAvatarPrefix(stringIndex, &info) != 0) {
                    void* text;
                    if ((info.flags >> 31) == 0)
                        text = reinterpret_cast<char*>(&info) + 1;
                    else
                        text = info.data;
                    SoundManager_AcquireSoundBank(g_soundManager, text);
                    SMarker dA;
                    if (AsCGolfPlayer(playerBytes)->loadedSoundNameCount <
                        AsCGolfPlayer(playerBytes)->loadedSoundNameCapacity) {
                        FxString* pos = *reinterpret_cast<FxString**>(&AsCGolfPlayer(playerBytes)->loadedSoundNames) +
                                        AsCGolfPlayer(playerBytes)->loadedSoundNameCount;
                        if (pos) {
                            try {
                                unsigned int flags = info.flags;
                                if ((flags >> 31) == 0) {
                                    unsigned int length = info.length;
                                    pos->flags = flags;
                                    pos->length = length;
                                    pos->data = info.data;
                                } else {
                                    pos->flags = 0;
                                    pos->length = 0;
                                    pos->data = 0;
                                    String_ReserveCapacity(pos, info.length);
                                    SAlloc volatile terminator;
                                    SAlloc term2;
                                    term2.value = terminator.value;
                                    String_ReplaceRange(pos, 0, 0, info.data, info.data + info.length, &term2);
                                }
                            } catch (...) {
                                typedef void ThrowFunction(const char*, void*, void*) __attribute__((noreturn));
                                ((ThrowFunction*)__throw)(0, 0, 0);
                            }
                        }
                        ++AsCGolfPlayer(playerBytes)->loadedSoundNameCount;
                    } else {
                        SMarker dB;
                        SMarker dC;
                        if (0x15555555u - AsCGolfPlayer(playerBytes)->loadedSoundNameCapacity < 1)
                            SFail(golfPlayerStrings + kTextVectorLengthError);
                        unsigned int need = AsCGolfPlayer(playerBytes)->loadedSoundNameCount + 1 -
                                            AsCGolfPlayer(playerBytes)->loadedSoundNameCapacity;
                        SoundNameReplacementStorage temp(reinterpret_cast<unsigned int*>(&loadedSoundNameCapacity));
                        unsigned int cap = AsCGolfPlayer(playerBytes)->loadedSoundNameCapacity;
                        if (need > 0x15555555u - cap)
                            SFail(golfPlayerStrings + kTextVectorLengthError);
                        if (cap < 0x071C71C7u) {
                            unsigned int growth = (cap + 1) * 3 / 5;
                            cap += SMax(growth, need);
                        } else if (cap < 0x0E38E38Eu) {
                            unsigned int growth = (cap + 1) / 2;
                            cap += SMax(growth, need);
                        } else
                            cap = 0x15555555u;
                        SMarker dD;
                        if (cap > 0x15555555u)
                            SFail(golfPlayerStrings + kTextVectorLengthError);
                        FxString* np = static_cast<FxString*>(appMallocDummy__FUl(cap * 12));
                        unsigned int off = AsCGolfPlayer(playerBytes)->loadedSoundNameCount;
                        temp.data = np;
                        temp.capacity = cap;
                        temp.offset = off;
                        SMarker dE;
                        FxString* pos = np + off + temp.size;
                        for (unsigned int cnt = 1; cnt != 0; --cnt, ++pos) {
                            if (pos) {
                                try {
                                    unsigned int flags = info.flags;
                                    if ((flags >> 31) == 0) {
                                        unsigned int length = info.length;
                                        pos->flags = flags;
                                        pos->length = length;
                                        pos->data = info.data;
                                    } else {
                                        pos->flags = 0;
                                        pos->length = 0;
                                        pos->data = 0;
                                        String_ReserveCapacity(pos, info.length);
                                        SAlloc volatile terminator;
                                        SAlloc term2;
                                        term2.value = terminator.value;
                                        String_ReplaceRange(pos, 0, 0, info.data, info.data + info.length, &term2);
                                    }
                                } catch (...) {
                                    typedef void ThrowFunction(const char*, void*, void*) __attribute__((noreturn));
                                    ((ThrowFunction*)__throw)(0, 0, 0);
                                }
                            }
                            ++temp.size;
                        }
                        FxString* ob = *reinterpret_cast<FxString**>(&AsCGolfPlayer(playerBytes)->loadedSoundNames);
                        int n = (reinterpret_cast<char*>(ob + AsCGolfPlayer(playerBytes)->loadedSoundNameCount) -
                                 reinterpret_cast<char*>(ob)) /
                                12;
                        SMarker dF;
                        temp.offset -= n;
                        unsigned int copyBytes = n * 12;
                        memcpy(temp.data + temp.offset, ob, copyBytes);
                        memset(ob, 0, copyBytes);
                        temp.size += n;
                        SSwap(playerBytes, temp);
                        FxString* start = temp.data + temp.offset;
                        FxString* e = start + temp.size;
                        while (e > start) {
                            --e;
                            if (e && (e->flags >> 31) != 0)
                                appFreeUni__FPv(e->data);
                        }
                        temp.size = 0;
                    }
                }
            }
        } else {
            *reinterpret_cast<char*>(*reinterpret_cast<char**>(&AsCGolfPlayer(playerBytes)->animationStates) +
                                     playerIndex * kAnimationStateStride + 0x38) = 0;
            void* property = g_sceneManager;
            const char* name = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, (unsigned char)playerIndex);
            DrawScene_SetNamedObjectPuppet(property, name, 0);
        }
    }
    CacheSwingPlacements(-1);
    void* slots;
    for (int k = 0; slots = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
             k < (int)*reinterpret_cast<unsigned char*>(static_cast<char*>(slots) + 0x2B9);
         ++k) {
        if (*reinterpret_cast<int*>(static_cast<char*>(GolfPlayer_GetEntryByIndex(slots, (unsigned char)k)) + 8) != 3)
            CacheShotKeyTimes(k);
    }
    if (!Sing_805DC7B0()) {
        if (!s_renderPacketInitialized) {
            RenderPacket_Init((void*)s_renderPacket);
            __register_global_object((void*)s_renderPacket, (void*)RenderPacket_Destroy,
                                     (void*)s_renderPacketDtorRegistration_golfPlayer);
            s_renderPacketInitialized = 1;
        }
        Sing_805DC7B0() = (void*)s_renderPacket;
    }
    GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
        g_shadowManager, golfPlayerStrings + kTextChar, kFloat_0_05_PlayerShadow, 0x10, golfPlayerStrings + kTextSun,
        static_cast<char*>(Sing_805DC7B0()) + 0x4C);
    if (!Sing_805DC7B0()) {
        if (!s_renderPacketInitialized) {
            RenderPacket_Init((void*)s_renderPacket);
            __register_global_object((void*)s_renderPacket, (void*)RenderPacket_Destroy,
                                     (void*)s_renderPacketDtorRegistration_golfPlayer);
            s_renderPacketInitialized = 1;
        }
        Sing_805DC7B0() = (void*)s_renderPacket;
    }
    GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
        g_shadowManager, golfPlayerStrings + kTextClub, kFloat_0_05_PlayerShadow, 0x100, golfPlayerStrings + kTextSun,
        static_cast<char*>(Sing_805DC7B0()) + 0x4C);
}
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Reload through the original virtual load callback.
void CGolfPlayer::OnReload() {
    struct VTable {
        virtual void method0() DECOMP_LOCAL_VIRTUAL;
        virtual void method1() DECOMP_LOCAL_VIRTUAL;
        virtual void method2() DECOMP_LOCAL_VIRTUAL;
        virtual void method3() DECOMP_LOCAL_VIRTUAL;
        virtual void method4() DECOMP_LOCAL_VIRTUAL;
        virtual void method5() DECOMP_LOCAL_VIRTUAL;
    };
    reinterpret_cast<VTable*>(this)->method3();
}

// Cache the first *shot key time for each of the sixteen swing animation names.
int CGolfPlayer::CacheShotKeyTimes(int player) {
    float epsilon;
    float rate;
    rate = kAnimationFramesPerSecond_GolfPlayer;
    epsilon = kGolfPlayerNearZeroTolerance;
    int end = 0;
    int start = 0;
    int frame;
    // No matching *shot key leaves that slot's previous value unchanged; return
    // is always zero. Stored values are (keyFrame - firstFrame) / framesPerSecond.
    for (unsigned int i = 0; i < kSwingAnimationCount; ++i) {
        float* key = ObjectList_FindAnimationRangeByName((char*)playerModels + player * kPlayerModelStride,
                                                         s_swingAnimationNames[i], 1, 0);
        if (key) {
            TargetIntStore((float)ceil(rate * key[0]), start);
            float endFrame = rate * key[1];
            double diff = TargetAbs((float)floor(endFrame) - endFrame);
            if (TargetLess(diff, epsilon))
                end = TargetInt(rate * key[1]) - 1;
            else
                end = TargetInt((float)floor(rate * key[1]));
            for (frame = start; frame < end; ++frame) {
                int index;
                if (WPuppet_FindKeyRange(ObjectList_GetSelected((char*)playerModels + player * kPlayerModelStride),
                                         frame, frame, &index, &index)) {
                    key = (float*)index;
                    void* object = ObjectList_GetSelected((char*)playerModels + player * kPlayerModelStride);
                    const char* text = *(const char**)((char*)((WPuppet*)object)->keyRecords + (int)key * 12 + 8);
                    if (strstr(text, golfPlayerStrings + kTextShot)) {
                        ((PlayerAnimationRecord*)animationStates)[player].shotKeyTimesSeconds[i] =
                            TargetFloat(frame - start) / rate;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

// Play a swing sound and conditionally start its character-specific companion sound.
void CGolfPlayer::PlaySwingSound(void* playerObject, int playerIndex, void* soundName) {
    Sound_PlayByName(g_soundManager, soundName, 0, 0, 0, 0, kFloat_0_5_GolfPlayer, kFloat_200_GolfSound,
                     kFloat_1_GolfPlayer);
    {
        unsigned char* mgr = reinterpret_cast<unsigned char*>(g_sharedDoc);
        void* player = (*(void**)&AsCSharedDoc(mgr)->golfDoc);
        unsigned char slot = AsCGolfDoc(player)->currentPlayer;
        unsigned char* rec = mgr + slot * 0x1D4;
        if (!*reinterpret_cast<int*>(rec + 0x1DC) && !AsCGolfDoc(player)->alternateMotionPowerDecided &&
            !AsCGolfDoc(player)->shotReactionActive && !AsCGolfDoc(player)->puttingReactionPending) {
            int soundSuppressed = 0;
            IActor_DispatchMessage(playerObject, playerObject, golfPlayerStrings + kTextGolfRule, 0x2D,
                                   (unsigned int)(&soundSuppressed), 0, 0, 0);
            if (!soundSuppressed)
                Sound_PlayByNameWithParameters(
                    g_soundManager,
                    *reinterpret_cast<void**>(GolfPlayer_GetEntryByIndex(
                        *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(g_sharedDoc) + 0xB90), playerIndex)),
                    soundName, 0, 0, 0, 0, kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
        }
    }
}

// Clear swing/trail state, copy scene lighting, and allocate the four player poses.
void CGolfPlayer::OnInit() {
    swingAnimationName[0] = 0;
    shotAnimationPrefix[0] = 0;
    trailEnabled[0] = 0;
    trailEnabled[1] = 0;
    trailEnabled[2] = 0;

    trailEnabled[3] = 0;
    ambientLightColor = (void*)Color_ScaleRGB((unsigned int)(*(void**)(s_sceneLighting + 0x14)), kFloat_1_GolfPlayer);
    *(WVector*)&lightDirection = *(WVector*)((s_sceneLighting + 4));
    void* old = playerStates;
    if (old) {
        __dla__FPv((unsigned char*)old - 0x10);
    }
    playerStates = 0;
    void* a = __nwa__FUl(0x1A0);
    if (a)
        a = __construct_new_array(a, (void*)PlayerPoseState_Init, 0, 0x64, 4);
    playerStates = a;
    animationDisabled = 0;
}

// Initialize one enabled pose with identity contact matrix and no bone overrides.
void PlayerPoseState_Init(void* self) {
    unsigned char* p = (unsigned char*)self;
    unsigned int z = 0;
    unsigned int one = 1;
    Matrix_SetIdentityAffine(p + 0x10);
    Vec3_Zero(p + 0x40);
    float zero = kFloat_0_GolfPlayer;
    ((PlayerPoseState*)p)->positionOverrideEnabled = z;
    ((PlayerPoseState*)p)->headRotation = zero;
    *(unsigned int*)&((PlayerPoseState*)p)->headBone = z;
    ((PlayerPoseState*)p)->groundSlopeRadians = zero;
    *(unsigned int*)&((PlayerPoseState*)p)->footBones[1] = z;
    *(unsigned int*)&((PlayerPoseState*)p)->footBones[0] = z;
    *(unsigned int*)&((PlayerPoseState*)p)->enabled = one;
    *(unsigned int*)&((PlayerPoseState*)p)->turnedThisFrame = z;
    *(unsigned int*)&((PlayerPoseState*)p)->turnedLastFrame = z;
    ((PlayerPoseState*)p)->headingRadians = zero;
}

// This view records only accesses used by processing; phase 2 and sign of word68
// gate translucency input. The gameplay meaning of word68 remains unresolved.
struct PlayerBallProcessView {
    char prefix[0x59];
    unsigned char phase;
    char unknown5A[0xE];
    int word68;
};
typedef char PlayerBallProcess_word68[((int)DECOMP_OFFSETOF(PlayerBallProcessView, word68) == 0x68) ? 1 : -1];
// Requested mode lives on CGolfPlayer (+2D4); committed mode lives on ShotInstanceView (+44).
// The Screen writes the request. Completion of entering-power animation commits it.
// Cancellation clears committed mode and refreshes Spot/Camera before resetting ready animation.
// Screen messages are synchronous: output pointers are borrowed only for dispatch.
enum PlayerScreenMessage {
    kScreenSetPowerBlend = 9,         // IN float*: interpolation ratio.
    kScreenQueryState = 0x30,         // OUT int*: only state 2 accepts toggle input.
    kScreenQueryPowerAction = 0x34,   // OUT int*: nonzero requests a power change.
    kScreenQueryRequestedPower = 0x35 // OUT int*: writes requestedPowerShotMode.
};
enum PlayerProcessMessage {
    kBallSetClubCategoryFlag = 9,
    kSpotRefresh = 5,
    kCameraRefresh = 12
};
enum {
    kPlayerBallInputPhase = 2,
    kPlayerScreenInputState = 2,
    kRequestedPowerModeOffset = (int)DECOMP_OFFSETOF(CGolfPlayer, requestedPowerShotMode),
    kCommittedPowerModeOffset = (int)DECOMP_OFFSETOF(ShotInstanceView, powerShotMode)
};
// Poll the logical input action, handle power-shot transitions, and advance animations/trails.
void CGolfPlayer::OnProcess(float deltaSeconds) {
    // Phase 1: poll the async avatar and skip processing while its active slot
    // is loading; then handle translucency and requested power-mode changes.
    register float deltaTime = deltaSeconds;
    int screenState;
    float ratio;
    float eventZero;
    int powerActionRequested;
    int waitingForActiveAvatar;
    if (!avatarPreloadPending)
        waitingForActiveAvatar = 0;
    else {
        ((*(C708Virtual**)&avatarPreloader))->update();
        if (((*(C708Virtual**)&avatarPreloader))->done())
            avatarPreloadPending = 0;
        waitingForActiveAvatar = preloadPlayerIndex == ((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))[0x3C1];
    }
    if (waitingForActiveAvatar)
        return;
    if (processingSuspended)
        return;
    {
        unsigned char* ballState = (unsigned char*)GolfBall__Fv();
        if (((PlayerBallProcessView*)ballState)->phase == kPlayerBallInputPhase) {
            C708Event(this, kScreenQueryState, &screenState);
            ballState = (unsigned char*)GolfBall__Fv();
            if (((PlayerBallProcessView*)ballState)->word68 >= 0)
                translucencyEnabled = 0;
            else {
                if (!((GameInputManager*&)GetInputManagerSlot())) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(&s_gameInputManager);
                        __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                                 &s_playerInputManagerDtorRegistration);
                        s_inputManagerInitialized = 1;
                    }
                    ((GameInputManager*&)GetInputManagerSlot()) = &s_gameInputManager;
                }
                // Logical action query: physical Wii Remote mapping is owned by the input manager.
                if (Input_IsActionPressed((void*)((GameInputManager*&)GetInputManagerSlot()),
                                          kActionTogglePlayerTranslucency, kAnyInputPlayer) &&
                    screenState == kPlayerScreenInputState)
                    translucencyEnabled = !translucencyEnabled;
            }
        }
        IActor_DispatchMessage(
            this, this, (char*)&golfPlayerStrings + kTextBall, kBallSetClubCategoryFlag,
            (unsigned int)((void*)(((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == kClubPutter)), 0,
            0, 0);
        powerActionRequested = 0;
        C708Event(this, kScreenQueryPowerAction, &powerActionRequested);
        if (CItemWindow::IsClosed()) {
            unsigned int mode = (*(unsigned int*)&AsCGolfDoc((unsigned char*)g_sharedDoc->golfDoc)->gameStateFlags);
            if ((mode == 8 || mode == 0x200 || mode == 0x40000 || mode == 0x100) && powerActionRequested) {
                C708Event(this, kScreenQueryRequestedPower, &requestedPowerShotMode);
                if (requestedPowerShotMode)
                    UpdatePowerShotPreparation(this);
                else if (*(int*)((unsigned char*)InstanceTable_Get() + kCommittedPowerModeOffset)) {
                    Sound_StopByAlias(g_soundManager, (char*)&golfPlayerStrings + kTextPowershotReady);
                    Sound_PlayByName(g_soundManager, (char*)&golfPlayerStrings + kTextPowershotCancel, 0, 0, 0, 0,
                                     kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
                    eventZero = kFloat_0_GolfPlayer;
                    *(int*)((unsigned char*)InstanceTable_Get() + kCommittedPowerModeOffset) = 0;
                    IActor_DispatchMessage(this, this, (char*)&golfPlayerStrings + kTextSpot, kSpotRefresh, 0, 0, 0, 0);
                    IActor_DispatchMessage(this, this, (char*)&golfPlayerStrings + kTextCamera, kCameraRefresh, 0, 0, 0,
                                           0);
                    C708Event(this, kScreenSetPowerBlend, &eventZero);
                    ResetReadyAnimation(this);
                }
            }
        }
#define MANAGER ((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))
#define ENTRY(off) ((*(unsigned char**)&animationStates) + (off))
        unsigned char* golfDocBytes = MANAGER;
        int activePlayerIndex = golfDocBytes[0x3C1];
        unsigned char* activeAnimation = ENTRY(activePlayerIndex * kAnimationStateStride);
        // Phase 2: the current slot's mode drives power-ready interpolation and
        // sidestep completion. Modes: 0 ready, 2 swinging, 3 entering power, 4 power ready.
        switch (((PlayerAnimationRecord*)activeAnimation)->mode) {
        case kAnimationEnteringPowerReady: {
            unsigned char* rangeBytes = *(unsigned char**)(activeAnimation + 4);
            ratio = (deltaTime + *(float*)activeAnimation - *(float*)rangeBytes) /
                    (*(float*)(rangeBytes + 4) - *(float*)rangeBytes);
            if (ratio > kFloat_1_GolfPlayer) {
                ratio = kFloat_1_GolfPlayer;
                ((PlayerAnimationRecord*)ENTRY(MANAGER[0x3C1] * kAnimationStateStride))->mode = kAnimationPowerReady;

                *(int*)((unsigned char*)InstanceTable_Get() + kCommittedPowerModeOffset) = (requestedPowerShotMode);
                IActor_DispatchMessage(this, this, (char*)golfPlayerStrings + kTextSpot, kSpotRefresh, 0, 0, 0, 0);
                IActor_DispatchMessage(this, this, (char*)golfPlayerStrings + kTextCamera, kCameraRefresh, 0, 0, 0, 0);
                GolfTask_NoOpStatus();
            }
            C708Event(this, kScreenSetPowerBlend, &ratio);
            break;
        }
        case kAnimationReady:
        case kAnimationPowerReady: {
            unsigned char* state = (*(unsigned char**)&playerStates) + activePlayerIndex * kPlayerPoseStride;
            if ((*(unsigned int*)&AsCGolfDoc(golfDocBytes)->gameStateFlags) & 0xA4000) {
                *(int*)(state + 4) = 0;
                *(int*)(8 + state) = 0;
            } else {
                if (*(int*)(state + 4) && !*(int*)(state + 8) && *(signed char*)shotAnimationPrefix) {
                    const char* name;
                    switch (*(int*)((unsigned char*)InstanceTable_Get() + kCommittedPowerModeOffset)) {
                    case 0:
                        name = (const char*)(int)String_FormatTemporary((char*)golfPlayerStrings + kTextReadyFormat,
                                                                        shotAnimationPrefix);
                        break;
                    case 1:
                        name = (const char*)(int)String_FormatTemporary(
                            (char*)golfPlayerStrings + kTextPowerReadyEndFormat, shotAnimationPrefix);
                        break;
                    case 2:
                        if ((*(unsigned char**)((unsigned char*)InstanceTable_Get() + 4))[0xA])
                            name = (const char*)(int)String_FormatTemporary(
                                (char*)golfPlayerStrings + kTextPowerReadyEndFormat, shotAnimationPrefix);
                        else
                            name = (const char*)(int)String_FormatTemporary(
                                (char*)golfPlayerStrings + kTextDoublePowerReadyEndFormat, shotAnimationPrefix);
                        break;
                    }
                    CGolfPlayer_SetPlayerAnimation(this, name, kFloat_0_GolfPlayer, -1);
                }
                *(int*)(state + 4) = *(int*)(state + 8);
                *(int*)(state + 8) = 0;
            }
            break;
        }
        }
        // Phase 3: advance each visible loaded animation in seconds, dispatch its
        // elapsed key events, and choose queued/ready/loop ranges on completion.
        int index = 0;
        int animationOffset = 0;
        int modelOffset = 0;
        unsigned char* strings = (unsigned char*)&golfPlayerStrings;
        float zero = kFloat_0_GolfPlayer;
        float one = kFloat_1_GolfPlayer;
        float idleRestartDistance = kIdleRestartDistance_GolfPlayer;
        struct {
            int completionArguments[8];
            unsigned char trailStart[12], trailEnd[12];
            char readyAnimationName[64];
        } buffers;
#define completionArguments buffers.completionArguments
#define trailStart buffers.trailStart
#define trailEnd buffers.trailEnd
#define readyAnimationName buffers.readyAnimationName
        for (; index < playerIndexOffset + MANAGER[0x2B9];
             animationOffset += kAnimationStateStride, modelOffset += kPlayerModelStride, ++index) {
            if (!playerIndexOffset &&
                *(int*)((unsigned char*)GolfPlayer_GetEntryByIndex(MANAGER, (unsigned char)index) + 8) == 3)
                continue;
            unsigned char* animationBytes = ENTRY(animationOffset);
            if (!animationBytes[0x38])
                continue;
            if (!ResourceModel801B_HasModelAndParts((*(unsigned char**)&playerModels) + modelOffset))
                continue;
            animationBytes = ENTRY(animationOffset);
            *(float*)animationBytes += deltaTime;
            animationBytes = ENTRY(animationOffset);
            float opacityVelocity = ((GolfPlayerAnimationState*)animationBytes)->interpolationRatePerSecond;
            if ((opacityVelocity > zero ? opacityVelocity : -opacityVelocity) > zero) {
                float opacity = ((GolfPlayerAnimationState*)animationBytes)->interpolationStart +
                                deltaTime * ((GolfPlayerAnimationState*)animationBytes)->interpolationRatePerSecond;
                if (opacity < zero)
                    opacity = zero;
                else if (opacity > one)
                    opacity = one;
                ((GolfPlayerAnimationState*)animationBytes)->interpolationStart = opacity;
            }
            float currentTime = *(float*)ENTRY(animationOffset);
            unsigned int animationRangeAddress = *(int*)(ENTRY(animationOffset) + 4);
            float previousTime = currentTime - deltaTime;
            void* selectedPuppet = ObjectList_GetSelected((*(unsigned char**)&playerModels) + modelOffset);
            AnimationEventParser_ProcessFrameRange(&sub_vtbl, selectedPuppet, previousTime, currentTime,
                                                   (const void*)(animationRangeAddress), index, 0);
            animationBytes = ENTRY(animationOffset);
            void* animationRange = *(void**)(animationBytes + 4);
            if (!animationRange)
                continue;
            if (!(*(float*)((unsigned char*)animationRange + 4) < *(float*)animationBytes))
                continue;
            float end = *(float*)animationRange;
            if (end != *(float*)((unsigned char*)animationRange + 4)) {
                if (((GolfPlayerAnimationState*)animationBytes)->animationMode == kAnimationSwinging) {
                    trailEnabled[0] = 0;
                    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)(strings + kTextTail), 0);
                    trailEnabled[1] = 0;
                    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)(strings + kTextTailRainbow), 0);
                    trailEnabled[2] = 0;
                    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)(strings + kTextTailPink), 0);
                    trailEnabled[3] = 0;
                    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)(strings + kTextTailBlue), 0);
                }
                animationBytes = ENTRY(animationOffset);
                if ((*(signed char*)&((GolfPlayerAnimationState*)animationBytes)->pendingAnimationName)) {
                    if (animationCompletionNotifyPending) {
                        animationCompletionNotifyPending = 0;
                        completionArguments[0] = 0;
                        completionArguments[1] = 0;
                        completionArguments[2] = 0;
                        completionArguments[3] = 0;
                        completionArguments[4] = 0;
                        completionArguments[5] = 0;
                        completionArguments[6] = 0;
                        completionArguments[7] = 0;
                        if (this)
                            ((C708Virtual*)this)->notify(completionArguments);
                    } else {
                        CGolfPlayer_SetPlayerAnimation(this, (const char*)(animationBytes + 0x39),
                                                       *(float*)animationBytes -
                                                           *(float*)((unsigned char*)*(void**)(animationBytes + 4) + 4),
                                                       index);
                        *(unsigned char*)(ENTRY(animationOffset) + 0x39) = 0;
                    }
                    continue;
                }
                {
                    sprintf(readyAnimationName, (const char*)(strings + kTextReadyFormat),
                            (const char*)(shotAnimationPrefix));
                    animationBytes = ENTRY(animationOffset);
                    void* rangeBytes = *(void**)(animationBytes + 4);
                    void* rangeName = *(void**)&((PlayerAnimationRange*)rangeBytes)->name;
                    if (!strcmp((const char*)rangeName, (const char*)(strings + kTextEntranceAnimation)) &&
                        *(signed char*)shotAnimationPrefix) {
                        void* nextAnimationName = *(void**)&((PlayerAnimationRange*)rangeBytes)->nextName;
                        if (strcmp(readyAnimationName, (const char*)nextAnimationName)) {
                            CGolfPlayer_SetPlayerAnimation(
                                this, readyAnimationName,
                                *(float*)animationBytes - *(float*)((unsigned char*)rangeBytes + 4), index);
                            continue;
                        }
                        CGolfPlayer_SetPlayerAnimation(this, (const char*)nextAnimationName,
                                                       *(float*)animationBytes -
                                                           *(float*)((unsigned char*)*(void**)(animationBytes + 4) + 4),
                                                       index);
                        readyForQueuedAnimation = 1;
                        continue;
                    }
                    int skip = 0;
                    if ((unsigned int)(((PlayerAnimationRange*)rangeBytes)->lastKey -
                                       ((PlayerAnimationRange*)rangeBytes)->firstKey) == 1 &&
                        !strcmp((const char*)rangeName, (const char*)(strings + kTextMTSTORYADDITION05)))
                        skip = 1;
                    if (skip)
                        continue;
                    int slot = index;
                    if (slot < 0)
                        slot = MANAGER[0x3C1];
                    unsigned char* state = (unsigned char*)playerStates + slot * kPlayerPoseStride;
                    if (index == MANAGER[0x3C1] && !strcmp((const char*)rangeName, readyAnimationName) &&
                        (*(int*)(state + 8) ||
                         *(int*)((unsigned char*)InstanceTable_Get() + kCommittedPowerModeOffset) ||
                         *(unsigned int*)(MANAGER + 0x358) != 8 || distanceToHole < idleRestartDistance)) {
                        animationBytes = ENTRY(animationOffset);
                        CGolfPlayer_SetPlayerAnimation(
                            this, (const char*)*(void**)(*(unsigned char**)(animationBytes + 4) + 0x18),
                            *(float*)animationBytes - *(float*)((unsigned char*)*(void**)(animationBytes + 4) + 4),
                            index);
                    } else {
                        animationBytes = ENTRY(animationOffset);
                        CGolfPlayer_SetPlayerAnimation(
                            this, (const char*)*(void**)(*(unsigned char**)(4 + animationBytes) + 0x1C),
                            *(float*)animationBytes - *(float*)((unsigned char*)*(void**)(animationBytes + 4) + 4),
                            index);
                        readyForQueuedAnimation = 1;
                    }
                }
                continue;
            } else {
                *(float*)animationBytes = end;
                *(unsigned int*)(ENTRY(animationOffset) + 0x5C) &= ~2u;
            }
        }
        // Phase 4: equipment changes rebuild attached effects/contact; each enabled
        // trail then samples resource attachment points into WORLD space for a segment.
        if ((*(int*)((unsigned char*)InstanceTable_Get() + 0x4C) & 2) != 0) {
            RebuildAttachedEffects(MANAGER[0x3C1], (void*)(1), (void*)(1));
            ResetReadyAnimation(this);
            UpdateSwingContactData(this, -1);
            unsigned char* animationBytes = ENTRY(MANAGER[0x3C1] * kAnimationStateStride);
            ((GolfPlayerAnimationState*)animationBytes)->flags |= (kAnimationTransformDirty | kAnimationClubLightDirty);
            ((C708Virtual*)this)->refresh();
        }
// Advance one enabled channel and append world endpoints. Keep this expansion:
// a forced-inline helper changes .text and anonymous cleanup relocation names.
#define CHANNEL(n)                                                                                      \
    if (trailEnabled[n]) {                                                                              \
        ((CTailEffect*)(tailEffects[n]))->UpdateLifetimes(deltaTime);                                   \
        unsigned char a[12], b[12];                                                                     \
        unsigned char* pair = (*(unsigned char**)&trailStartBoneEntry);                                 \
        Matrix_TransformPoint(a, *(unsigned char**)((pair + 4)) + 0x18, *(unsigned char**)pair + 0x78); \
        *(WVector*)trailStart = *(WVector*)a;                                                           \
        pair = (*(unsigned char**)&trailEndBoneEntry);                                                  \
        Matrix_TransformPoint(b, *(unsigned char**)(4 + pair) + 0x18, *(unsigned char**)pair + 0x78);   \
        *(WVector*)trailEnd = *(WVector*)b;                                                             \
        ((CTailEffect*)(tailEffects[n]))->AddSegment(trailStart, trailEnd);                             \
    }
        CHANNEL(0)
        CHANNEL(1)
        CHANNEL(2)
        CHANNEL(3)
#undef CHANNEL
#undef ENTRY
#undef MANAGER
#undef completionArguments
#undef trailStart
#undef trailEnd
#undef readyAnimationName
    }
}

// Read the selected club category from the instance table (wood/iron/wedge/putter).
unsigned char InstanceTable_GetClubCategory(void* instanceTable) {
    return ((ShotInstanceView*)instanceTable)->selectedClub->category;
}

// Read the power-shot mode as the legacy pointer-sized word used by callers.
void* InstanceTable_GetPowerShotModeWord(void* instanceTable) {
    return *(void**)((char*)&AsCGolfPlayer(instanceTable)->swingReach);
}

// Consume cutscene slot commands or update the active player for the current fade/game state.
void CGolfPlayer::OnDisplay() {
    // In scripted state 0x01000000, consume pending slot/position/heading and
    // animation-queue commands before updating each model. Position is world
    // space; queued heading is DEGREES converted to radians (unlike HandleMsg).
    // Outside scripted state, fade state gates active/all-player model updates.
    unsigned char* globals = (unsigned char*)g_sharedDoc;
    unsigned char* manager = (*(unsigned char**)&AsCSharedDoc(globals)->golfDoc);
    if ((*(unsigned int*)&AsCGolfDoc(manager)->gameStateFlags) & kGameStateScriptedPose) {
        unsigned char* event = U8013C5CC_Event();
        if (*(int*)(event + 4)) {
            *(int*)(event + 4) = 0;
            unsigned short id = *(unsigned short*)event;
            if (id != 0xffff)
                AddPlayerSlot(id); /* 0x8013D43C-0x8013D44C: r4 = id */
            else {
                playerIndexOffset = playerIndexOffset - 1;
                int index = playerIndexOffset + GetGolfDocBytes()[0x2B9];
                SharedStringState_DefaultHook(index);
                unsigned char* ev = (unsigned char*)SlotTable_GetEntryByte(U8013C5CC_Event(), index);
                *(int*)(ev + 4) = 0;
                *(int*)(ev + 8) = 0;
                SetSlotEnabled(index, 0, 1);
            }
        }
        float degrees;
        float zero;
        float one;
        int index;
        int row;
        one = kFloat_1_GolfPlayer;
        index = 0;
        zero = kFloat_0_GolfPlayer;
        row = 0;
        degrees = kDegreesToRadians_GolfPlayer;
        for (; index < (int)(playerIndexOffset + GetGolfDocBytes()[0x2B9]); row += kAnimationStateStride, ++index) {
            unsigned char* ev = (unsigned char*)SlotTable_GetEntryByte(event, index);
            // Per-slot phase 1: consume an optional world-position override.
            if (*(int*)(ev + 0x10)) {
                *(int*)(ev + 0x10) = 0;
                void* field = *(void**)(ev + 0x0C);
                struct U8013C5CC_Vec3 vec = *(struct U8013C5CC_Vec3*)(ev + 0x1C);
                int slot = ResolvePlayerIndex(index);
                unsigned char* state = GetPlayerPoseRecord(this, slot);
                __as__7WVectorFRC7WVector(state + 0x40, &vec);
                *(void**)(state + 0x4C) = field;
                unsigned int m1[3];
                Vec3_Zero(m1);
                unsigned int m2[3];
                Vec3_Zero(m2);
                float scale[3];
                scale[0] = one;
                scale[1] = zero;
                scale[2] = one;
                unsigned int t1[3];
                __ml__FfRC7WVector(t1, scale, kFloat_16_0);
                unsigned int o1[3];
                __mi__FRC7WVectorRC7WVector(o1, &vec, t1);
                __as__7WVectorFRC7WVector(m1, o1);
                scale[0] = one;
                scale[1] = zero;
                scale[2] = one;
                unsigned int t2[3];
                __ml__FfRC7WVector(t2, scale, kFloat_16_0);
                unsigned int o2[3];
                __pl__FRC7WVectorRC7WVector(o2, &vec, t2);
                void* handle = __as__7WVectorFRC7WVector(m2, o2);
                unsigned int packed[6];
                packed[0] = m1[0];
                packed[1] = m1[1];
                packed[2] = m1[2];
                packed[3] = m2[0];
                packed[4] = m2[1];
                packed[5] = m2[2];
                ((CQuadTree*)CQuadTree_GetInstance())->GetObjList((float*)packed);
                unsigned char* obj = (*(unsigned char**)&animationStates) + slot * kAnimationStateStride;
                *(unsigned int*)(obj + 0x5C) |= (kAnimationTransformDirty | kAnimationAttachmentDirty |
                                                 kAnimationSamplePose | kAnimationPositionDirty);
            }
            // Per-slot phase 2: rebuild heading/contact from a queued degree angle.
            if (*(int*)(ev + 0x14)) {
                *(int*)(ev + 0x14) = 0;
                int slot = ResolvePlayerIndex(index);
                float angle = degrees * (*(float*)(ev + 0x2C));
                unsigned char* p10 = GetPlayerPoseRecord(this, slot) + 0x10;
                unsigned char* pa = GetPlayerPoseRecord(this, slot) + 0x0C;
                unsigned char* p = GetPlayerPoseRecord(this, slot);
                *(int*)(p + 0x08) = 0;
                *(int*)(p + 0x04) = 0;
                *(float*)pa = angle;
                unsigned char rotationMatrix[0x30]; // 3x4 matrix written by Matrix_SetAxisRotation
                Matrix_SetAxisRotation(rotationMatrix, angle, 1);
                Matrix_CopyAffine(p10, rotationMatrix);
                UpdateSwingContactData(this, slot);
                unsigned char* obj = (*(unsigned char**)&animationStates) + slot * kAnimationStateStride;
                *(unsigned int*)(obj + 0x5C) |=
                    (kAnimationAttachmentDirty | kAnimationSamplePose | kAnimationPositionDirty);
            }
            // Per-slot phase 3: start/queue one animation and shift owned strings;
            // the vacated tail string is cleared and its heap storage released.
            if (*(int*)(ev + 0x18)) {
                if (*(int*)(ev + 0x38)) {
                    unsigned char* item = *(unsigned char**)(ev + 0x34);
                    unsigned char* text;
                    if (!item[0]) {
                        if (*(unsigned int*)(item + 8) >> 31)
                            text = *(unsigned char**)(item + 0x10);
                        else
                            text = item + 9;
                        CGolfPlayer_SetPlayerAnimation(this, (const char*)text, kFloat_0_GolfPlayer, index);
                    } else {
                        unsigned char* obj = (*(unsigned char**)&animationStates) + row;
                        if (!*(signed char*)(obj + 0x39)) {
                            if (*(unsigned int*)(item + 8) >> 31)
                                text = *(unsigned char**)(item + 0x10);
                            else
                                text = item + 9;
                            strcpy((char*)(obj + 0x39), (const char*)text);
                        }
                    }
                    struct U8013C5CC_Elem* begin = (struct U8013C5CC_Elem*)*(void**)(ev + 0x34);
                    struct U8013C5CC_Elem* end = begin + *(int*)(ev + 0x38);
                    if ((end - begin) - 1 != 0) {
                        struct U8013C5CC_Elem* d = begin;
                        struct U8013C5CC_Elem* s = begin + 1;
                        while (s < end) {
                            AssignAnimationCommand((unsigned char*)d, (unsigned char*)s);
                            ++d;
                            ++s;
                        }
                    }
                    int count = *(int*)(ev + 0x38);
                    unsigned char* base = *(unsigned char**)(ev + 0x34);
                    count = count - 1;
                    unsigned char* last = base + count * 0x14;
                    *(int*)(ev + 0x38) = count;
                    if (last) {
                        last[0] = 0;
                        *(int*)(last + 4) = 0;
                        if (!(*(unsigned int*)(last + 8) >> 31)) {
                            last[9] = 0;
                            last[8] = (unsigned char)(last[8] & 0xFFFFFF80);
                        } else {
                            *(unsigned char*)*(unsigned char**)(last + 0x10) = 0;
                            *(int*)(last + 0xC) = 0;
                        }
                        if (last + 8) {
                            if (*(unsigned int*)(last + 8) >> 31)
                                appFreeUni__FPv(*(void**)(last + 0x10));
                        }
                    }
                } else if (readyForQueuedAnimation) {
                    *(int*)(ev + 0x18) = 0;
                }
            }
            UpdatePlayerModelForDisplay(index);
        }
    } else {
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(&s_fadeOverlay);
                __register_global_object(&s_fadeOverlay, (void*)ResourceState_Dtor,
                                         &s_fadeOverlayDtorRegistration_GolfPlayer);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = &s_fadeOverlay;
        }
        if (FadeState_GetState(Sing_805DC608()) != 2) {
            unsigned char* mgr = GetGolfDocBytes();
            unsigned int flags = *(unsigned int*)(mgr + 0x358);
            if ((flags & 0x20000) || (((unsigned char*)g_sharedDoc)[0x8C0] == 1 && (flags & 0x3000)))
                DrawPlayerModels();
            else
                UpdatePlayerModelForDisplay(mgr[0x3C1]);
        }
    }
}

#pragma opt_propagation off
// Update visible player models, lighting, head icons, and attached effect transforms.
void CGolfPlayer::DrawPlayerModels() {
    if (avatarPreloader != 0)
        return;
#define ENTRY(off) (((unsigned char*)animationStates) + (off))
#define OBJ(off) (((unsigned char*)playerModels) + (off))
    unsigned char* strings;
    int entryOffset;
    int objectOffset;
    unsigned char* row;
    int i;
    float a[12];
    int data[3];
    WVector b;

    row = reinterpret_cast<unsigned char*>(this);
    i = 0;
    entryOffset = 0;
    strings = (unsigned char*)&golfPlayerStrings;
    objectOffset = 0;
    for (; i < (int)GetGolfDocBytes()[0x2B9];
         entryOffset += kAnimationStateStride, objectOffset += kPlayerModelStride, row += 0x40, ++i) {
        if (!ENTRY(entryOffset)[0x38])
            continue;
        void* state = GolfPlayer_GetEntryByIndex(GetGolfDocBytes(), (unsigned char)i);
        if (*(int*)((unsigned char*)state + 8) == 3)
            continue;
        void* node = ObjectList_GetSelected(OBJ(objectOffset));
        WPuppet_SetAnimationTimeFromRanges(node, 0, *(float*)ENTRY(entryOffset), 0, 0);
        WPuppet_UpdateSkinFromAnimationSource(PlayerModel_GetPuppet(OBJ(objectOffset)), node);
        WPuppet_UpdateBound(PlayerModel_GetPuppet(OBJ(objectOffset)), 1);
        void* visual = PlayerModel_GetPuppet(OBJ(objectOffset));
        if (WView_DefaultCapabilityTrueA(g_view, &((WPuppet*)visual)->boundsCenter))
            PlayerModel_ApplyBoneRelativeLighting(OBJ(objectOffset), &lightType, 0, 0);
        if (g_headIcon) {
            void* layout =
                WPuppet_FindOrCacheBone(PlayerModel_GetPuppet(OBJ(objectOffset)), (const char*)(strings + 0x1BA));
            struct I3 {
                int v[3];
            };
            *(I3*)data = *(I3*)(&((PuppetBoneView*)layout)->worldMatrix.position);
            ((CHeadIcon*)g_headIcon)->ProjectPlayerPoint(i, data);
        }
        unsigned int current;
        unsigned char* ent;
        ent = ENTRY(entryOffset);
        current = *(unsigned int*)(ent + 0xA4);
        if (current && current != *(unsigned int*)(ent + 0xA8)) {
            *(unsigned int*)(ent + 0xA8) = current;
            AvatarModel_DefaultReturnOneHook(OBJ(objectOffset));
        }
        if (!clubPuppet)
            return;
        unsigned char* manager;
        unsigned char* globals = (unsigned char*)g_sharedDoc;
        manager = (*(unsigned char**)&AsCSharedDoc(globals)->golfDoc);
        if (!(*(unsigned int*)(manager + 0x358) & 0x3000))
            continue;
        if (globals[0x8C0] == 1) {
            if (i != ((CGolfDoc*)manager)->ResolveTurnAfterMatchHoleOut())
                continue;
            if (*(unsigned int*)(ENTRY(entryOffset) + 0x5C) & kAnimationClubHidden)
                continue;
        }
        if (*(unsigned int*)(ENTRY(entryOffset) + 0x5C) & kAnimationUseLockedAttachment) {
            Model_SetMatrixRecursive(clubPuppet, 0, &attachmentDirection);
        } else {
            unsigned char* name;
            void* thing;
            name = (unsigned char*)(golfPlayerStrings + kTextClub);
            thing = WPuppet_FindNamedBoneEntry(PlayerModel_GetPuppet(OBJ(objectOffset)), (const char*)name);
            Matrix_CopyAffine(a, &((BoneAttachmentEntry*)thing)->bone->worldMatrix);
            Matrix_TransformPoint(&b, &((BoneAttachmentEntry*)thing)->attachment->position,
                                  &((BoneAttachmentEntry*)thing)->bone->worldMatrix);
            *(WVector*)(a + 9) = b;
            unsigned char* sel = (unsigned char*)InstanceTable_Get();
            WVector d = (*(float*)(row + ((*(unsigned char**)(sel + 4))[0xA] << 4) + 0x134)) * *(WVector*)a;
            *(WVector*)(a + 9) += d;
            Model_SetMatrixRecursive(clubPuppet, 0, a);
        }
        WPuppet_UpdateBoneWorldMatrices(clubPuppet, 0, 0, kFloat_1_GolfPlayer);
        WPuppet_UpdateBoundsFromBone(clubPuppet);
        ((U8013C5CC_Callable*)clubPuppet)->v3(&lightType, 0, 0);
    }
#undef ENTRY
#undef OBJ
}
#pragma opt_propagation reset

// Copy a three-component vector as words, retaining the original bit patterns.
void Vector_CopyThreeWords(unsigned int* dest, const unsigned int* source) {
    dest[0] = source[0];
    dest[1] = source[1];
    dest[2] = source[2];
}

#pragma push
#pragma opt_loop_invariants off
#pragma opt_common_subs off
// Parse equipped-item effect tags, remove old effects, and attach enabled character/club effects.
void CGolfPlayer::RebuildAttachedEffects(int index, void* clearExistingEffects, void* includeClubEffects) {
    // index is a concrete slot, not a negative-current sentinel. The two void*
    // arguments carry boolean words. IDs in the two vectors are non-owning effect
    // handles: remove effects through CFx, then clear counts (retain allocations).
    char prefix[5] = {(char)s_characterEffectTagPrefixByte0, (char)s_characterEffectTagPrefixByte1,
                      (char)s_characterEffectTagPrefixByte2, (char)s_characterEffectTagPrefixByte3,
                      (char)s_characterEffectTagPrefixByte4};
    char effectPath[0x40];
    int sharedPlayerOffset = index * 0x1D4;

    ResourceParams params;
    char* global = (char*)SharedDoc_GetCurrent() + sharedPlayerOffset;
    void* manager = InstanceTable_Get();
    void* clubModels = ClubModel_FindCachedForItem(manager, *(void**)(global + 0x1CC));
    FxAttachment_Init(&params);
    // Phase 1: remove recorded effects before parsing the replacement metadata.
    if (clearExistingEffects) {
        for (unsigned int i = 0; i < Vector_GetCount((char*)&attachedSprayIds); ++i) {
            unsigned int* p = Array_GetElementPtr((unsigned int**)this + 7, i);
            ((CFx*)(CFx::GetInstance()))->RemoveSpray(*p);
        }
        for (unsigned int i = 0; i < Vector_GetCount((char*)&attachedSequenceIds); ++i) {
            unsigned int* p = Array_GetElementPtr2((unsigned int**)this + 10, i);
            ((CFx*)(CFx::GetInstance()))->RemoveSequence(*p, 1);
        }
        AttachedSprayIds_Clear((char*)&attachedSprayIds);
        AttachedSequenceIds_Clear((char*)&attachedSequenceIds);
    }
    // Phase 2: parse character bone tags. '&id' tokens gate creation on equipped
    // items; .spr/.seq choose the registry. params borrows bone/world matrices.
    int playerModelOffset = index * kPlayerModelStride;
    ResourceTable* playerPuppet = (ResourceTable*)PlayerModel_GetPuppet(((char*)playerModels) + playerModelOffset);
    int boneEntryOffset = 0;
    for (unsigned int n = 0; n < playerPuppet->count; ++n, boneEntryOffset += 8) {
        ResourceEntry* rec = (ResourceEntry*)((char*)playerPuppet->entries + boneEntryOffset);
        if (!rec || !rec->info->text || !rec->node || PuppetBone_IsAttachedEffectSuppressed(rec->node))
            continue;
        char* cursor = rec->info->text;
        for (;;) {
            char suffix[0x40];
            char* found = strstr(cursor, prefix);
            if (!found)
                break;
            cursor = String_ParseQuotedToken(found + strlen(prefix), effectPath);
            int length = strlen(effectPath);
            if (!length || length >= 0x40)
                continue;
            char* dot = CString::FindLastCharForwarder(effectPath, '.');
            if (!dot || strlen(dot) < 4)
                continue;
            cursor = String_ParseQuotedToken(cursor, suffix);
            length = strlen(suffix);
            if (!length || length >= 0x40)
                continue;
            int equippedKeyFound = 0;
            int requiresEquippedKey = 0;
            while (!equippedKeyFound && suffix[0] == '&') {
                requiresEquippedKey = 1;
                void* key = String_ParseDecimalInt(((suffix + 1)));
                void* current = PlayerModel_GetEquippedItemKeyList(((char*)playerModels) + playerModelOffset);
                if (ResolvedItemSelection_ContainsPart(current, key)) {
                    equippedKeyFound = 1;
                    break;
                }
                int keyOffset = 0;
                for (int j = 0; j < 5; ++j, keyOffset += 4) {
                    if (*(unsigned int*)((char*)SharedDoc_GetCurrent() + (keyOffset + sharedPlayerOffset) + 0x190) ==
                        (unsigned int)key) {
                        equippedKeyFound = 1;
                        break;
                    }
                }
                if (equippedKeyFound)
                    break;
                char* list;
                if (clubModels) {
                    if ((list = *(char**)clubModels) &&
                        (unsigned int)key ==
                            *(unsigned int*)(list + 0x70 +
                                             (unsigned char)InstanceTable_GetClubCategory(InstanceTable_Get()) * 4)) {
                        equippedKeyFound = 1;
                        break;
                    }
                }
                cursor = String_ParseQuotedToken(cursor, suffix);
                length = strlen(suffix);
                if (!length || length >= 0x40)
                    break;
            }
            if (requiresEquippedKey && equippedKeyFound == 0)
                continue;
            params.transform = PuppetBone_GetWorldMatrix(rec->node);
            params.extra = rec->info->transform;
            if (String_CompareCString(dot + 1, golfPlayerStrings + kTextSpr) == 0) {
                void* made =
                    ((CFx*)(CFx::GetInstance()))->CreateAttachedEffect((Vec2_8010*)(&params), effectPath, 0, 0, 0);
                AttachedSprayIds_Append((IntVector*)((char*)&attachedSprayIds), (unsigned int*)&made);
            } else if (String_CompareCString(dot + 1, golfPlayerStrings + kTextSeq) == 0) {
                void* made =
                    ((CFx*)(CFx::GetInstance()))->CreateAttachedEffect((Vec2_8010*)(&params), effectPath, 0, 0, 0);
                AttachedSequenceIds_Append((IntVector*)((char*)&attachedSequenceIds), (unsigned int*)&made);
            }
        }
    }
    // Phase 3: course-specific breath effect uses a rotated bone basis while
    // preserving its world translation in the per-player breath matrix.
    if (SharedDoc_IsCourse8Or13()) {
        ResourceEntry* key = (ResourceEntry*)WPuppet_FindNamedBoneEntryForward(
            PlayerModel_GetPuppet(((char*)playerModels) + playerModelOffset), golfPlayerStrings + kTextBreathe);
        if (key) {
            char matrix[0x30];
            Matrix_SetAxisRotation(matrix, kBreathEffectYaw_GolfPlayer, 1);
            index *= 0x30;
            void* transform = (char*)this + index + 0x2E8;
            Matrix_CopyAffine(transform, PuppetBone_GetWorldMatrix(key->node));
            WVector savedPosition;
            void* position = (char*)this + index + 0x30C;
            Vector_CopyThreeWords((unsigned int*)(&savedPosition), (const unsigned int*)(position));
            char zero[0xC];
            *(WVector*)position =
                *(WVector*)(Vec3_Set(zero, kFloat_0_GolfPlayer, kFloat_0_GolfPlayer, kFloat_0_GolfPlayer));
            Matrix_ConcatAffineAssign(transform, matrix);
            ResourceParams special;
            *(WVector*)position = savedPosition;
            FxAttachment_Init(&special);
            special.transform = transform;
            special.extra = key->info->transform;
            void* made =
                ((CFx*)(CFx::GetInstance()))
                    ->CreateAttachedEffect((Vec2_8010*)(&special), golfPlayerStrings + kTextBreathSeq, 1, 0, 0);
            AttachedSequenceIds_Append((IntVector*)((char*)&attachedSequenceIds), (unsigned int*)&made);
        }
    }
    // Phase 4: optionally parse the selected club's tags into the same ID lists.
    if (clubModels) {
        if (includeClubEffects) {
            char prefix2[4] = {(char)s_clubEffectTagPrefixByte0, (char)s_clubEffectTagPrefixByte1,
                               (char)s_clubEffectTagPrefixByte2, (char)s_clubEffectTagPrefixByte3};

            ResourceTable* table =
                *(ResourceTable**)((char*)clubModels +
                                   ((unsigned char)InstanceTable_GetClubCategory(InstanceTable_Get())) * 4 + 4);
            for (unsigned int q = 0; q < table->count; ++q) {
                ResourceEntry* rec = q + table->entries;
                if (!rec || !rec->info->text || !rec->node || PuppetBone_IsAttachedEffectSuppressed(rec->node))
                    continue;
                char* cursor = rec->info->text;
                for (;;) {
                    cursor = strstr(cursor, prefix2);
                    if (!cursor)
                        break;
                    cursor = String_ParseSpaceDelimitedToken(cursor + strlen(prefix2), effectPath);
                    int length = strlen(effectPath);
                    if (!length || length >= 0x40)
                        continue;
                    char* dot = CString::FindLastCharForwarder(effectPath, '.');
                    if (!dot || strlen(dot) < 4)
                        continue;
                    params.transform = PuppetBone_GetWorldMatrix(rec->node);
                    params.extra = rec->info->transform;
                    if (String_CompareCString(1 + dot, golfPlayerStrings + kTextSpr) == 0) {
                        void* t = CFx::GetInstance();
                        void* made = ((CFx*)t)->CreateAttachedEffect((Vec2_8010*)(&params), effectPath, 0, 0, 0);
                        AttachedSprayIds_Append((IntVector*)((char*)&attachedSprayIds), (unsigned int*)&made);
                    } else if (String_CompareCString(dot + 1, golfPlayerStrings + kTextSeq) == 0) {
                        void* t = CFx::GetInstance();
                        void* made = ((CFx*)t)->CreateAttachedEffect((Vec2_8010*)(&params), effectPath, 0, 0, 0);
                        AttachedSequenceIds_Append((IntVector*)((char*)&attachedSequenceIds), (unsigned int*)&made);
                    }
                }
            }
        }
    }
}
#pragma pop

// Test the bone flag that suppresses attached-effect creation.
int PuppetBone_IsAttachedEffectSuppressed(void* ptr) {
    return (((PuppetBoneView*)ptr)->flags >> 18) & 1;
}

// Return the model's equipped-item-key container.
void* PlayerModel_GetEquippedItemKeyList(void* ptr) {
    return ((EquippedItemKeysView*)ptr)->equippedItemKeys;
}

// Return an element of the attached-spray ID vector.
unsigned int* Array_GetElementPtr(unsigned int** ptr, unsigned int index) {
    return ptr[0] + index;
}

// Return an element of the attached-sequence ID vector.
unsigned int* Array_GetElementPtr2(unsigned int** ptr, unsigned int index) {
    return ptr[0] + index;
}

// Clear the spray-ID vector size while retaining its allocated storage.
void AttachedSprayIds_Clear(void* ptr) {
    char** field = (char**)((char*)ptr + 4);
    char* value = *field;
    *field = (char*)(value - (char*)*(unsigned int*)field);
}

// Clear the sequence-ID vector size while retaining its allocated storage.
void AttachedSequenceIds_Clear(void* ptr) {
    char** field = (char**)((char*)ptr + 4);
    char* value = *field;
    *field = (char*)(value - (char*)*(unsigned int*)field);
}

#pragma push
#pragma use_lmw_stmw on
// Append an attached-spray ID, growing the vector with exception-safe ownership transfer.
void AttachedSprayIds_Append(IntVector* self, unsigned int* value) {
    unsigned int max = 0x3FFFFFFF;
    // Fast path writes one ID. Slow path stages the new tail ID, copies old IDs,
    // zeroes old storage, swaps allocations and lets the guard free the old block.
    // The alias temporaries below preserve compiler-generated cleanup symbols.
    unsigned int initialCapacity = self->capacity;
    unsigned int savedSize = self->size;
    unsigned int size = savedSize;
    if (size < initialCapacity) {
        unsigned int savedSize2 = size;
        unsigned int savedValue = 1 + savedSize2;
        unsigned int nextSize = savedValue;
        unsigned int savedNextSize = nextSize;
        self->data[savedNextSize - 1] = *value;
        unsigned int savedNextSize2 = nextSize;
        self->size = savedNextSize2;
        return;
    }

    if (max - initialCapacity < 1)
        targetThrow(golfPlayerStrings + kTextVectorLengthError);

    // Reserve phase: the guard will own new storage after the tail is initialized.
    // self still owns [data, data+size); value may alias that range.
    TargetStorage storage(&self->capacity);
    unsigned int oldSize = ((volatile IntVector*)self)->size;
    unsigned int savedOldSize = oldSize;
    int oldCapacityValue = ((volatile IntVector*)self)->capacity;
    int savedOldCapacityValue = oldCapacityValue;
    unsigned int savedValue2 = 1 + savedOldSize;
    unsigned int needed = savedValue2 - savedOldCapacityValue;
    unsigned int capacity = self->capacity;
    if (needed > max - capacity)
        targetThrow(golfPlayerStrings + kTextVectorLengthError);

    if (capacity < 0x15555555) {
        unsigned int savedCapacity = capacity;
        unsigned int savedValue3 = savedCapacity + 1;
        int threeFifths;
        threeFifths = (savedValue3) * 3 / 5;
        capacity += targetMax(threeFifths, needed);
    } else if (!(!(capacity < 0x2AAAAAAA))) {
        unsigned int half;
        unsigned int savedCapacity2 = capacity;
        unsigned int savedValue4 = savedCapacity2 + 1;
        half = (savedValue4) / 2;
        unsigned int savedHalf = half;
        capacity += targetMax(savedHalf, needed);
    } else {
        unsigned int savedMax = max;
        capacity = savedMax;
    }
    if (capacity > max)
        targetThrow(golfPlayerStrings + kTextVectorLengthError);
    unsigned int savedCapacity3 = capacity;
    unsigned int savedValue5 = savedCapacity3 * 4;
    // Construct tail before copying the prefix, so an aliased input ID survives.
    unsigned int* newBegin = (unsigned int*)appMallocDummy__FUl(savedValue5);
    __typeof__(newBegin + storage.start) uncommittedBegin = newBegin + storage.start;
    __typeof__(newBegin + storage.start) tailPointerTypeProbe = uncommittedBegin;
    unsigned int savedSize3 = self->size;
    storage.start = savedSize3;
    __typeof__(storage.start) oldElementCount = storage.start;
    __typeof__(storage.start + newBegin) newTailBegin = storage.start + newBegin;
    __typeof__(newBegin + oldElementCount) newTailBeginCopy = newTailBegin;
    __typeof__(tailPointerTypeProbe) stagedTailBegin = newTailBeginCopy;
    __typeof__(storage.size) stagedCountBeforeAppend = storage.size;
    (stagedTailBegin)[stagedCountBeforeAppend] = *value;
    __typeof__(storage.size) stagedCountForCommit = storage.size;
    storage.data = newBegin;
    unsigned int savedCapacity4 = capacity;
    storage.capacity = savedCapacity4;
    __typeof__(1 + stagedCountForCommit) stagedCountAfterAppend = stagedCountForCommit + 1;
    storage.size = stagedCountAfterAppend;
    __typeof__(storage.data) allocationTypeProbe = storage.data;
    __typeof__(storage.data) stagedAllocation = storage.data;
    unsigned int* savedData = self->data;
    __typeof__(storage.data) stagedAllocationCopy = stagedAllocation;
    __typeof__(allocationTypeProbe) prefixAllocationTypeProbe = stagedAllocationCopy;
    unsigned int savedSize4 = self->size * 4;
    __typeof__(storage.data) prefixAllocation = storage.data;
    unsigned int* last = (unsigned int*)((char*)savedData + savedSize4);
    __typeof__(storage.data) prefixAllocationCopy = prefixAllocation;
    __typeof__(storage.data) prefixAllocationCursor = prefixAllocationCopy;
    unsigned int* savedData2 = self->data;
    __typeof__(storage.data) prefixTypeProbe = storage.data;
    __typeof__(prefixTypeProbe) prefixDestination = prefixAllocationCursor;
    __typeof__(prefixAllocationTypeProbe) newAllocationBase = prefixDestination;

    // Copy phase: source [first,last) contains old IDs; the new tail is already live.
    unsigned int* first = savedData2;
    int count = last - first;
    int savedCount = count;
    __typeof__(storage.start) oldPrefixStartProbe = storage.start;
    __typeof__(oldPrefixStartProbe) oldPrefixStart = storage.start;
    __typeof__(newAllocationBase + storage.start) oldPrefixEnd = storage.start + newAllocationBase;
    __typeof__(storage.start + newAllocationBase) oldPrefixEndCopy = oldPrefixEnd;
    int savedCount2 = count;
    storage.start = oldPrefixStart - savedCount2;
    __typeof__(storage.start) newPrefixStart = storage.start;
    __typeof__(storage.start) newPrefixStartCopy = newPrefixStart;
    __typeof__(storage.start + newAllocationBase) newPrefixBegin = storage.start + newAllocationBase;
    __typeof__(oldPrefixEndCopy) newPrefixBeginCopy = newPrefixBegin;
    __typeof__(newAllocationBase + storage.start) prefixBeginAddress = newAllocationBase + storage.start;
    __typeof__(newAllocationBase + storage.start) prefixBeginAddressCopy = prefixBeginAddress;
    __typeof__(prefixBeginAddressCopy) prefixBeginComputed = newPrefixStartCopy + newAllocationBase;
    __typeof__(prefixBeginComputed) prefixBeginTypeProbe = newPrefixBeginCopy;
    __typeof__(prefixBeginTypeProbe) prefixCopyDestination = newAllocationBase + storage.start;
    int savedValue6 = savedCount * 4;
    memcpy(prefixCopyDestination, first, savedValue6);
    int savedCount3 = count;
    int savedValue7 = 4 * savedCount3;
    __typeof__(storage.size) constructedCountProbe = storage.size;
    __typeof__(constructedCountProbe) constructedCountCopy = storage.size;
    __typeof__(constructedCountCopy) constructedCountAtClear = storage.size;
    memset(first, 0, savedValue7);
    int savedCount4 = count;
    __typeof__(constructedCountAtClear) constructedTailCount = storage.size;
    storage.size = savedCount4 + constructedTailCount;
    unsigned int savedCapacity5 = self->capacity;
    unsigned int* savedData3 = self->data;
    unsigned int* old = savedData3;

    // Commit phase: self takes the new allocation; guard takes the old block.
    // Reset guard size after exchange so its destructor frees storage, not live IDs.
    self->size = 0;
    self->capacity = storage.capacity;
    int oldCapacity = savedCapacity5;
    int savedOldCapacity = oldCapacity;
    storage.capacity = savedOldCapacity;
    self->data = storage.data;
    storage.data = old;
    self->size = storage.size;
    storage.size = 0;
}

// Append an attached-sequence ID, growing the vector with exception-safe ownership transfer.
void AttachedSequenceIds_Append(IntVector* self, unsigned int* value) {
    int bytes;
    // Same ownership phases as AttachedSprayIds_Append: vectors own their word
    // allocations, while each stored word remains an externally managed effect ID.
    TargetVector868* pointers = (TargetVector868*)self;
    if (pointers->size < pointers->capacity) {
        int nextSize = 1 + self->size;

        pointers->data[nextSize - 1] = (void*)*value;
        pointers->size = nextSize;
        return;
    }
    if (0x3FFFFFFF - self->capacity < 1U)
        targetThrow868(golfPlayerStrings + kTextVectorLengthError);

    unsigned int oldSize = ((volatile IntVector*)self)->size;
    unsigned int oldCapacityValue = ((volatile IntVector*)self)->capacity;

    TargetStorage storage(&self->capacity);
    unsigned int needed = oldSize + 1 - oldCapacityValue;
    unsigned int capacity = targetCapacity868(self, needed);

    unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
    storage.start = self->size;
    (data + storage.start)[storage.size] = *value;
    int constructed = (storage.size + 1);
    unsigned int* first = self->data;
    unsigned int* last = (unsigned int*)((char*)first + 4 * self->size);
    storage.data = data;
    int count;

    storage.capacity = capacity;
    bytes = (char*)last - (char*)first;
    count = bytes / 4;
    storage.size = constructed;
    storage.start -= count;
    memcpy(data + storage.start, first, count * 4);
    memset(first, 0, (count * 4));
    storage.size += count;
    unsigned int* old = self->data;
    unsigned int oldCapacity = self->capacity;
    self->size = 0;
    self->capacity = storage.capacity;
    storage.capacity = oldCapacity;
    self->data = storage.data;
    storage.data = old;
    self->size = storage.size;
    storage.size = 0;
}
#pragma pop

// Select the player's result-rank animation, falling back to the facial animation range.
void CGolfPlayer::SelectRankAnimation(unsigned int index) {
    ResourceString string;
    unsigned int offset = index * kAnimationStateStride;
    ((GolfPlayerAnimationState*)((char*)animationStates + offset))->animationMode = kAnimationReady;
    ((GolfPlayerAnimationState*)((char*)animationStates + offset))->pendingAnimationName = 0;

    void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned int rank = (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(manager, index))->rankFlags >> 4) & 15;
    unsigned int count;
    const char* rankAnimationName;
    if (rank == 1) {
        rankAnimationName = golfPlayerStrings + kTextFirstPlaceAnimation;
        count = !(string.storageHeader >> 31) ? *(unsigned char*)&string & 0x7F : string.lengthOrInlineBytes;
        TargetAllocator allocator;
        TargetAllocator value(allocator);
        String_ReplaceRange(&string, 0, count, rankAnimationName, rankAnimationName + strlen(rankAnimationName),
                            &value.value);
    } else {
        manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char playerCount = AsCGolfDoc(manager)->playerCount;
        rank = (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(manager, index))->rankFlags >> 4) & 15;
        if (rank == playerCount) {
            rankAnimationName = golfPlayerStrings + kTextThirdPlaceAnimation;
            count = !(string.storageHeader >> 31) ? *(unsigned char*)&string & 0x7F : string.lengthOrInlineBytes;
            TargetAllocator allocator;
            TargetAllocator value(allocator);
            String_ReplaceRange(&string, 0, count, rankAnimationName, rankAnimationName + strlen(rankAnimationName),
                                &value.value);
        } else {
            rank = (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                                      index))
                        ->rankFlags >>
                    4) &
                   15;
            if (rank == 2) {
                rankAnimationName = golfPlayerStrings + kTextSecondPlaceAnimation;
                count = !(string.storageHeader >> 31) ? *(unsigned char*)&string & 0x7F : string.lengthOrInlineBytes;
                TargetAllocator allocator;
                TargetAllocator value(allocator);
                String_ReplaceRange(&string, 0, count, rankAnimationName, rankAnimationName + strlen(rankAnimationName),
                                    &value.value);
            } else {
                rankAnimationName = golfPlayerStrings + kTextThirdPlaceAnimation;
                count = !(string.storageHeader >> 31) ? *(unsigned char*)&string & 0x7F : string.lengthOrInlineBytes;
                TargetAllocator allocator;
                TargetAllocator value(allocator);
                String_ReplaceRange(&string, 0, count, rankAnimationName, rankAnimationName + strlen(rankAnimationName),
                                    &value.value);
            }
        }
    }

    const char* name = !(string.storageHeader >> 31) ? (char*)&string + 1 : string.data;
    unsigned int entryOffset = index * kPlayerModelStride;
    ((GolfPlayerAnimationState*)((char*)animationStates + offset))->animationObject =
        (void*)ObjectList_FindAnimationRangeByName((char*)playerModels + entryOffset, name, 1, 0);
    if (!((GolfPlayerAnimationState*)((char*)animationStates + offset))->animationObject)
        ((GolfPlayerAnimationState*)((char*)animationStates + offset))->animationObject =
            (void*)ObjectList_FindAnimationRangeByName((char*)playerModels + entryOffset,
                                                       golfPlayerStrings + kTextMTFACIALANIMATION, 1, 0);
    char* entries = (char*)animationStates;
    *(float*)(entries + offset) = **(float**)(entries + offset + 4);
}

#pragma push
#pragma inline_depth(2)
enum PlayerPowerMode {
    kPlayerNormalPower = 0,
    kPlayerSinglePower = 1,
    kPlayerDoublePower = 2
};

// Choose the first turning pose from power mode, club category and current animation.
// Singleton/document calls intentionally repeat because each call can refresh shared state.
static inline void selectInitialTurnAnimation(CGolfPlayer* self, f32& screenTurnValue) {
    if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != kPlayerNormalPower &&
        ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category != kClubPutter) {
        char* player = (char*)self->animationStates +
                       AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer * kAnimationStateStride;
        if (((PlayerAnimationRecord*)player)->mode != kAnimationEnteringPowerReady) {
            if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == kPlayerSinglePower ||
                ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category != kClubWood)
                CGolfPlayer_SetPlayerAnimation((CGolfPlayer*)(self),
                                               String_FormatTemporary(golfPlayerStrings + kTextPowerSidestepFormat,
                                                                      AsCGolfPlayer(self)->shotAnimationPrefix),
                                               kFloat_0_GolfPlayer, -1);
            else
                CGolfPlayer_SetPlayerAnimation(
                    (CGolfPlayer*)(self),
                    String_FormatTemporary(golfPlayerStrings + kTextDoublePowerSidestepFormat,
                                           AsCGolfPlayer(self)->shotAnimationPrefix),
                    kFloat_0_GolfPlayer, -1);
        }
    } else {
        screenTurnValue = kFloat_0_GolfPlayer;
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextScreen, 9, (unsigned int)(&screenTurnValue), 0, 0,
                               0);
        ((PlayerAnimationRecord*)((char*)self->animationStates +
                                  AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer * kAnimationStateStride))
            ->mode = kAnimationReady;
        CGolfPlayer_SetPlayerAnimation(
            (CGolfPlayer*)(self),
            String_FormatTemporary(golfPlayerStrings + kTextSidestepFormat, AsCGolfPlayer(self)->shotAnimationPrefix),
            kFloat_0_GolfPlayer, -1);
    }
}

#pragma push
#pragma always_inline on
// Invoke the virtual callback used when this message's deadline has not arrived.
// The target's broader semantics are unresolved; keep its shared placeholder name.
static inline void invokeDeferredPlayerCommandCallback(CGolfPlayer* player) {
    ((PlayerDispatch*)player)->fn30();
}
// Invoke the callback selected by both the refresh message and the turning path.
static inline void invokePlayerDisplayRefreshCallback(CGolfPlayer* player) {
    ((PlayerDispatch*)player)->fnD0();
}
#pragma pop

// Outgoing actor-message ABI; the Screen scalar's broader interpretation is unresolved.
enum PlayerUiMessage {
    kSpotRefreshMessage = 5,
    kScreenValueMessage = 9
};
// Address-taken payloads and release temporaries stay in the caller for stack/register identity.

// Choose the power-ready animation and publish the Spot/Screen state values.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerSetPowerMode(CGolfPlayer* self, Cmd* cmd, f32& enabledScreenValue,
                                            f32& disabledScreenValue) {
    s32 requestedPowerMode = (s32)cmd->argument;
    if (requestedPowerMode != 0) {
        (*(u32*)&((PlayerAnimationRecord*)playerAnimationBytes(self, currentPlayerIndex()))->mode) =
            kAnimationPowerReady;
        enabledScreenValue = kFloat_1_GolfPlayer;
        ((ShotInstanceView*)InstanceTable_Get())->powerShotMode = requestedPowerMode;
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextSpot, kSpotRefreshMessage, (u32)(0), 0, 0, 0);
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextScreen, kScreenValueMessage,
                               (u32)(&enabledScreenValue), 0, 0, 0);
        const char* name;
        if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != 0 &&
            (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 1 ||
             ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category != 0))
            name = String_FormatTemporary(golfPlayerStrings + kTextPowerReadyEndFormat,
                                          AsCGolfPlayer(self)->shotAnimationPrefix);
        else
            name = String_FormatTemporary(golfPlayerStrings + kTextDoublePowerReadyEndFormat,
                                          AsCGolfPlayer(self)->shotAnimationPrefix);
        CGolfPlayer_SetPlayerAnimation((CGolfPlayer*)(self), name, kFloat_0_GolfPlayer, -1);
    } else {
        (*(u32*)&((PlayerAnimationRecord*)playerAnimationBytes(self, currentPlayerIndex()))->mode) = 0;
        disabledScreenValue = kFloat_0_GolfPlayer;
        ((ShotInstanceView*)InstanceTable_Get())->powerShotMode = kPlayerNormalPower;
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextSpot, kSpotRefreshMessage, (u32)(0), 0, 0, 0);
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextScreen, kScreenValueMessage,
                               (u32)(&disabledScreenValue), 0, 0, 0);
        CGolfPlayer_SetPlayerAnimation(
            (CGolfPlayer*)(self),
            String_FormatTemporary(golfPlayerStrings + kTextReadyFormat, AsCGolfPlayer(self)->shotAnimationPrefix),
            kFloat_0_GolfPlayer, -1);
    }
    return;
}

// Enter single-power ready mode, retaining the paired Spot notifications.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerSetPowerReady(CGolfPlayer* self, Cmd* cmd, f32& readyScreenValue) {
    CGolfPlayer_SetPlayerAnimation(
        (CGolfPlayer*)(self),
        String_FormatTemporary(golfPlayerStrings + kTextPowerReadyEndFormat, AsCGolfPlayer(self)->shotAnimationPrefix),
        kFloat_0_GolfPlayer, -1);
    (*(u32*)&((PlayerAnimationRecord*)playerAnimationBytes(self, currentPlayerIndex()))->mode) = kAnimationPowerReady;
    ((ShotInstanceView*)InstanceTable_Get())->powerShotMode = kPlayerSinglePower;
    IActor_DispatchMessage(self, self, golfPlayerStrings + kTextSpot, kSpotRefreshMessage, (u32)(0), 0, 0, 0);
    AsCGolfPlayer(self)->singlePowerShotReady = 1;
    AsCGolfPlayer(self)->doublePowerShotDisabled = 1;
    (*(u32*)&((PlayerAnimationRecord*)playerAnimationBytes(self, currentPlayerIndex()))->mode) = kAnimationPowerReady;
    ((ShotInstanceView*)InstanceTable_Get())->powerShotMode = kPlayerSinglePower;
    IActor_DispatchMessage(self, self, golfPlayerStrings + kTextSpot, kSpotRefreshMessage, (u32)(0), 0, 0, 0);
    readyScreenValue = kFloat_1_GolfPlayer;
    IActor_DispatchMessage(self, self, golfPlayerStrings + kTextScreen, kScreenValueMessage, (u32)(&readyScreenValue),
                           0, 0, 0);
    return;
}

// Load the selected club, publish its IFF ID and refresh puppet/shadow/placement state.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerSelectClub(CGolfPlayer* self, Cmd* cmd) {
    u32 clubItemId = (u32)cmd->argument;
    void* clubPuppetBundle = ClubModel_CreateForItem(InstanceTable_Get(), clubItemId);
    if (clubPuppetBundle == 0)
        return;
    unsignedFieldAt(((u32)g_sharedDoc), currentPlayerIndex() * (int)sizeof(PlayerEquipmentSlotView) +
                                            (int)DECOMP_OFFSETOF(PlayerEquipmentSlotView, clubItemId)) = clubItemId;
    (*(unsigned int*)&AsCGolfPlayer(self)->clubPuppet) =
        unsignedFieldAt(clubPuppetBundle, 4 + ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category * 4);
    if (cmd->argument2)
        ((CGolfPlayer*)self)->CacheSwingPlacements(0);
    DrawScene_SetNamedObjectPuppet((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor,
                                   (void*)((*(unsigned int*)&AsCGolfPlayer(self)->clubPuppet)));
    ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
        (void*)(((u32)g_shadowManager)), golfPlayerStrings + kTextClub, (*(char**)&AsCGolfPlayer(self)->clubPuppet));
    ((PlayerAnimationRecord*)playerAnimationBytes(self, currentPlayerIndex()))->flags |=
        (kAnimationTransformDirty | kAnimationAttachmentDirty | kAnimationClubLightDirty | kAnimationSamplePose |
         kAnimationPositionDirty);

    self->swingReach =
        self->byPlayerAndClub[currentPlayerIndex()][((ShotInstanceView*)InstanceTable_Get())->selectedClub->category]
            .offset.x;
    return;
}

// Queue the follow-through animation selected by the equipped item and power mode.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerQueueFollowThrough(CGolfPlayer* self, Cmd* cmd) {
    void* playerItemSelection = ((CGolfDoc*)currentGolfDocBytes())->GetItemSelection(currentPlayerIndex());
    if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != 0) {
        if ((unsignedFieldAt(playerItemSelection, 0) & 0x3ffffff) == 4) {
            if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 1)
                strcpy(playerAnimationBytes(self, currentPlayerIndex()) + 0x39,
                       golfPlayerStrings + kTextWoodPowerFollowThrough);
        } else if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 1)
            strcpy(playerAnimationBytes(self, currentPlayerIndex()) + 0x39, golfPlayerStrings + kTextWoodFollowThrough);
    } else
        strcpy(playerAnimationBytes(self, currentPlayerIndex()) + 0x39, golfPlayerStrings + kTextWoodFollowThrough);
    return;
}

// Release slot resources with mask 7 and detach its named scene entry.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerReleaseResources(CGolfPlayer* self, Cmd* cmd, u32& shadowManagerHandle,
                                                s32& playerSlot) {
    shadowManagerHandle = ((u32)g_shadowManager);
    playerSlot = (s32)cmd->argument;
    void* slotPuppet = PlayerModel_GetPuppet(playerModelBytes(self, playerSlot));
    GfxObjList_RemoveEntryFromNamedGroup(shadowManagerHandle, golfPlayerStrings + kTextChar, slotPuppet);
    PlayerModel_ReleaseResourcesByMask(playerModelBytes(self, playerSlot), 7);
    u32 sceneManager = (u32)g_sceneManager;
    const char* sceneEntryName = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, playerSlot);
    DrawScene_SetNamedObjectPuppet((void*)sceneManager, sceneEntryName, 0);
    return;
}

// Release slot resources with mask 2 and detach its named scene entry.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerReleaseModel(CGolfPlayer* self, Cmd* cmd, u32& shadowManagerHandle, s32& playerSlot) {
    shadowManagerHandle = ((u32)g_shadowManager);
    playerSlot = (s32)cmd->argument;

    void* slotPuppet = PlayerModel_GetPuppet(playerModelBytes(self, playerSlot));
    GfxObjList_RemoveEntryFromNamedGroup(shadowManagerHandle, golfPlayerStrings + kTextChar, slotPuppet);
    PlayerModel_ReleaseResourcesByMask(playerModelBytes(self, playerSlot), 2);
    u32 sceneManager = (u32)g_sceneManager;
    const char* sceneEntryName = String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, playerSlot);
    DrawScene_SetNamedObjectPuppet((void*)sceneManager, sceneEntryName, 0);
    return;
}

// If the slot is unloaded, choose the existing load-kind discriminator and start its preloader.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerBeginAvatarLoad(CGolfPlayer* self, Cmd* cmd) {
    if (ResourceModel801B_HasModelAndParts(playerModelBytes(self, (s32)cmd->argument)))
        return;
    s32 playerSlot = (s32)cmd->argument;
    (*(unsigned int*)&AsCGolfPlayer(self)->preloadPlayerIndex) = playerSlot;
    if ((s32)cmd->argument2 == 1)
        AsCGolfPlayer(self)->avatarPreloadKind = 4;
    else {
        s32 avatarLoadKind = 2;
        if (unsignedFieldAt(((u32)g_sharedDoc), playerSlot * 0x1d4 + 0x1dc))
            avatarLoadKind = 1;
        AsCGolfPlayer(self)->avatarPreloadKind = avatarLoadKind;
    }
    void* playerItemSelection = ((CGolfDoc*)currentGolfDocBytes())
                                    ->GetItemSelection((u8)(*(unsigned int*)&AsCGolfPlayer(self)->preloadPlayerIndex));
    AsCGolfPlayer(self)->avatarPreloader =
        CPreloadAvatarAsync::Create(playerItemSelection, (*(int*)&AsCGolfPlayer(self)->avatarPreloadKind), 0,
                                    (u32)AsCGolfDoc(currentGolfDocBytes())->playerCount <= 2);
    (*(unsigned int*)&AsCGolfPlayer(self)->avatarPreloadPending) = 1;
    return;
}

// Apply the borrowed renderer-option value to the requested slot puppet when present.
// Message/payload storage is borrowed synchronously; no payload pointer escapes.
static inline void handlePlayerSetPuppetState(CGolfPlayer* self, Cmd* cmd) {
    void* slotPuppet =
        PlayerModel_GetPuppet((char*)AsCGolfPlayer(self)->playerModels + (s32)cmd->argument2 * kPlayerModelStride);
    if (slotPuppet)
        WPuppet_SetRendererOption(slotPuppet, cmd->argument);
    return;
}

// Dispatch scheduled player commands for turning, swinging, models, visibility, and cutscene slots.
void CGolfPlayer::HandleMsg(Cmd* cmd) {
    // Each case documents its own input/output words and state transition.
    // Payload pointers are borrowed synchronously; callers own output storage.
    // A future deadline invokes fn30 without consuming this packet. Keep repeated
    // document/type probes: they can update shared caches between side effects.
    char* commandSupportStorage = s_playerInputManagerDtorRegistration;
    if (cmd->scheduledTimeMs > g_currentTimeMilliseconds) {
        invokeDeferredPlayerCommandCallback(this);
        return;
    }
    switch (cmd->message) {
    // IN argument=float* delta radians. Mark turning, rebuild contact, then sample/display the pose.
    case kPlayerTurnByAngle: {
        f32 temporary[12];
        PlayerPoseState* state =
            &((PlayerPoseState*)playerStates)[AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer];
        f32 screenTurnValue; // Caller-owned message payload; lives through synchronous dispatch.
        if (((PlayerPoseState*)state)->turnedLastFrame == 0) {
            selectInitialTurnAnimation(this, screenTurnValue);
        }
        ((PlayerPoseState*)state)->turnedThisFrame = 1;
        PlayerPoseState* rotation =
            &((PlayerPoseState*)playerStates)[AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer];
        double angle = (((PlayerPoseState*)rotation)->headingRadians += *(f32*)cmd->argument);
        void* matrix =
            ((PlayerPoseState*)playerStates)[AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer].contactMatrix;

        Matrix_SetAxisRotation(temporary, (f32)angle, 1);
        Matrix_CopyAffine(matrix, temporary);
        CGolfPlayer::UpdateSwingContactData(this, -1);
        char* player = (char*)animationStates +
                       AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer * kAnimationStateStride;
        ((PlayerAnimationRecord*)player)->flags |= kAnimationSamplePose;
        invokePlayerDisplayRefreshCallback(this);
        break;
    }
    // IN argument=value word. Exactly 1 enables translucency; all other words disable it.
    case kPlayerSetTranslucency:
        (*(signed char*)&translucencyEnabled) = (u32)cmd->argument == 1;
        break;
    // OUT argument=float* radians. Resolve match turn first; negative turn falls back to current player.
    case kPlayerGetHeading: {
        s32 player = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->ResolveTurnAfterMatchHoleOut();
        if (player < 0)
            player = AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer;
        *(f32*)cmd->argument = ((PlayerPoseState*)(*(unsigned int*)&playerStates))[player].headingRadians;
        break;
    }
    // IN argument=borrowed name; argument2=integer seconds converted to float, not float bits.
    case kPlayerSetAnimation: {
        CGolfPlayer_SetPlayerAnimation(this, (const char*)cmd->argument, (f32)(s32)cmd->argument2, -1);
        break;
    }
    // No payload. Transition to swing mode once; completion-pending prevents restarting it.
    case kPlayerStartSwing: {
        // Start once, then optionally queue long-wood/putt reactions from horizontal
        // ballForHistory-to-holeRecordBytes distances. The Y component is deliberately discarded.
        if (animationCompletionNotifyPending == 0) {
            void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            u8 activePlayer = AsCGolfDoc(golfDocument)->currentPlayer;
            char* activeAnimation = (char*)(*(unsigned int*)&animationStates) + activePlayer * kAnimationStateStride;
            if (((PlayerAnimationRecord*)activeAnimation)->mode != kAnimationSwinging) {
                f32 currentHolePosition[3];
                f32 horizontalDelta[3];
                ((PlayerAnimationRecord*)activeAnimation)->mode = kAnimationSwinging;
                (*(u8*)&((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))
                      ->pendingAnimationName[0]) = 0;
                CGolfPlayer_SetPlayerAnimation(this, swingAnimationName, kFloat_0_GolfPlayer, -1);
                if ((((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == kClubWood ||
                     ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category == kClubIron) &&
                    AsCGolfDoc(currentGolfDocBytes())->impactGrade < 2)
                    break;
                f32 currentBallToHole[3];
                char* holeRecordBytes = (*(char**)&AsCGolfDoc(currentGolfDocBytes())->holeRecords) +
                                        AsCGolfDoc(currentGolfDocBytes())->currentHoleIndex * 32;
                vectorFieldAt(currentHolePosition, 0) = vectorFieldAt(holeRecordBytes, 0xc);
                void* ballForHistory = GolfBall__Fv();
                void* historyPosition = PreData__9CGolfBallFi(GolfBall__Fv(), unsignedFieldAt(ballForHistory, 0x64));
                __mi__FRC7WVectorRC7WVector(currentBallToHole, historyPosition, currentHolePosition);
                *(WVector*)horizontalDelta = *(WVector*)currentBallToHole;
                horizontalDelta[1] = kFloat_0_GolfPlayer;
                f32 initialBallToHoleXZ;
                f32 currentBallToHoleXZ = (*(WVector*)horizontalDelta).Magnitude();
                // Repeat the hole/document lookup after querying current history: callees may
                // refresh shared state. History slot 0 supplies the initial-shot distance.
                f32 initialHolePosition[3];
                f32 initialBallToHole[3];
                holeRecordBytes = (*(char**)&AsCGolfDoc(currentGolfDocBytes())->holeRecords) +
                                  AsCGolfDoc(currentGolfDocBytes())->currentHoleIndex * 32;
                vectorFieldAt(initialHolePosition, 0) = vectorFieldAt(holeRecordBytes, 0xc);
                historyPosition = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
                __mi__FRC7WVectorRC7WVector(initialBallToHole, historyPosition, initialHolePosition);
                *(WVector*)horizontalDelta = *(WVector*)initialBallToHole;
                horizontalDelta[1] = kFloat_0_GolfPlayer;
                initialBallToHoleXZ = (*(WVector*)horizontalDelta).Magnitude();
                // Wood policy uses current distance; putt policy uses initial distance.
                // Reaction thresholds remain in game world units; Y is zeroed explicitly.
                switch (((ShotInstanceView*)InstanceTable_Get())->selectedClub->category) {
                case kClubWood:
                    if (byteFieldAt(GolfBall__Fv(), 0x58) == 0 &&
                        currentBallToHoleXZ > kLongWoodShotDistance_GolfPlayer)
                        sendPlayerEvent(this, 0x240, 0x1d, 0);
                    break;
                case kClubPutter:
                    if (initialBallToHoleXZ > kPuttFollowThroughDistance_GolfPlayer) {
                        s32 reactionChoice;
                        if (initialBallToHoleXZ > kFloat96_GolfPlayer)
                            reactionChoice = rand() % 4;
                        else if (initialBallToHoleXZ > kFloat64_GolfPlayer)
                            reactionChoice = rand() % 3;
                        else
                            reactionChoice = rand() % 2;
                        if (AsCGolfDoc(currentGolfDocBytes())->shotReactionActive == 0) {
                            if (currentBallToHoleXZ < kShortPuttDistance_GolfPlayer && 1 <= reactionChoice) {
                                sendPlayerEvent(this, 0x13a, 0x18, 0);
                                strcpy(playerAnimationBytes(this, currentPlayerIndex()) + 0x39,
                                       golfPlayerStrings + kTextPuttingFollowThrough);
                                (*(u32*)&AsCGolfDoc(currentGolfDocBytes())->puttingReactionPending) = 1;
                            } else
                                (*(u32*)&AsCGolfDoc(currentGolfDocBytes())->puttingReactionPending) = 0;
                        } else if (AsCGolfDoc(currentGolfDocBytes())->puttingReactionPending != 0) {
                            sendPlayerEvent(this, 0x13a, 0x18, 0);
                            strcpy(playerAnimationBytes(this, currentPlayerIndex()) + 0x39,
                                   golfPlayerStrings + kTextPuttingFollowThrough);
                        }
                    }
                    break;
                }
            }
        }
        break;
    }
    // No payload. Re-enter ready animation using the current club/power/turn state.
    case kPlayerResetShotAnimation:
        ResetReadyAnimation(this);
        break;
    // IN argument=float* radians. Clear turn flags; rebuild contact and mark pose/position dirty.
    case kPlayerSetHeading: {
        f32 angle = *(f32*)cmd->argument;
        s32 player = currentPlayerIndex();
        void* matrix = rotationState(this, player) + 0x10;
        char* rotation = rotationState(this, player);
        char* flags = rotationState(this, player);
        *(s32*)((8 + flags)) = 0;
        *(s32*)(((flags + 4))) = 0;
        *(f32*)(rotation + 0xc) = angle;
        f32 temporary[12];
        Matrix_SetAxisRotation(temporary, angle, 1);
        Matrix_CopyAffine(matrix, temporary);
        CGolfPlayer::UpdateSwingContactData(this, player);
        *(u32*)((char*)animationStates + player * kAnimationStateStride + 0x5c) |=
            (kAnimationAttachmentDirty | kAnimationSamplePose | kAnimationPositionDirty);
        break;
    }
    // IN argument=Vec3* world point. Copy override, query +/-16 horizontal bounds, mark display dirty.
    case kPlayerSetPosition: {
        f32 pos[3];
        vectorFieldAt(pos, 0) = vectorFieldAt(cmd->argument, 0);
        s32 index = currentPlayerIndex();
        char* rotation = rotationState(this, index);
        *(WVector*)(rotation + 0x40) = *(WVector*)pos;
        ((PlayerPoseState*)rotation)->positionOverrideEnabled = 1;
        f32 high[3];
        f32 unitMarginLow[3];
        f32 low[3];
        f32 marginLow[3];
        f32 lowCorner[3];
        f32 unitMarginHigh[3];
        f32 marginHigh[3];
        f32 highCorner[3];
        Vec3_Zero(low);
        Vec3_Zero(high);
        unitMarginLow[0] = kFloat_1_GolfPlayer;
        unitMarginLow[1] = kFloat_0_GolfPlayer;
        unitMarginLow[2] = kFloat_1_GolfPlayer;
        __ml__FfRC7WVector(marginLow, unitMarginLow, kFloat_16_0);
        __mi__FRC7WVectorRC7WVector(lowCorner, pos, marginLow);
        *(WVector*)low = *(WVector*)lowCorner;
        unitMarginHigh[0] = kFloat_1_GolfPlayer;
        unitMarginHigh[1] = kFloat_0_GolfPlayer;
        unitMarginHigh[2] = kFloat_1_GolfPlayer;
        __ml__FfRC7WVector(marginHigh, unitMarginHigh, kFloat_16_0);
        __pl__FRC7WVectorRC7WVector(highCorner, pos, marginHigh);
        *(WVector*)high = *(WVector*)highCorner;
        f32 bounds[6];
        vectorFieldAt(bounds, 0) = vectorFieldAt(low, 0);
        vectorFieldAt(bounds, 12) = vectorFieldAt(high, 0);
        ((CQuadTree*)CQuadTree_GetInstance())->GetObjList(bounds);
        ((PlayerAnimationRecord*)playerAnimationBytes(this, index))->flags |=
            (kAnimationTransformDirty | kAnimationAttachmentDirty | kAnimationSamplePose | kAnimationPositionDirty);
        break;
    }
    // OUT argument=Vec3* copied override position for the current player; no pointer escapes.
    case kPlayerGetPosition: {
        void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        *(WVector*)cmd->argument =
            *(WVector*)((char*)playerStates + AsCGolfDoc(manager)->currentPlayer * kPlayerPoseStride + 0x40);
        break;
    }
    // No payload. Enter power-ready mode and publish Spot/Screen state; repeated sends are intentional.
    case kPlayerSetPowerReady: {
        f32 readyScreenValue;
        handlePlayerSetPowerReady(this, cmd, readyScreenValue);
        break;
    }
    // IN argument=slot VALUE; OUT argument2=affine matrix. Head bone must exist.
    case kPlayerGetHeadMatrix: {
        void* part = WPuppet_FindOrCacheBone(PlayerModel_GetPuppet(playerModelBytes(this, (s32)cmd->argument)),
                                             golfPlayerStrings + kTextBip01Head);
        Matrix_CopyAffine(cmd->argument2, (char*)&((PuppetBoneView*)part)->worldMatrix);
        break;
    }
    // No payload. Reconcile slot visibility, contact, trail bones, distance, club and shadows.
    case kPlayerPrepareTurn: {
        // Reset slot/trail state, place contact relative to the ball, query nearby
        // world bounds, then bind the active character and selected club shadows.
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i)
            SetSlotEnabled(i, i == currentPlayerIndex(), 0);
        attachmentPositionLocked = 0;
        translucencyEnabled = 0;
        trailEnabled[0] = 0;
        animationCompletionNotifyPending = 0;
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextTail, 0);
        trailEnabled[1] = 0;
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextTailRainbow, 0);
        trailEnabled[2] = 0;
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextTailPink, 0);
        trailEnabled[3] = 0;
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextTailBlue, 0);
        ResetReadyAnimation(this);
        f32 delta[3];
        char* matrix = playerPoseBytes(this, currentPlayerIndex()) + 0x10;
        f32 pos[3];
        pos[1] = floatFieldAt(GolfBall__Fv(), 0xc0);
        pos[0] = kFloat_0_GolfPlayer;
        pos[2] = kFloat_0_GolfPlayer;
        __mi__FRC7WVectorRC7WVector(delta, GolfBall__Fv(), pos);
        *(WVector*)(matrix + 0x30) = *(WVector*)delta;
        f32 low[3];
        f32 unitMarginLow[3];
        f32 high[3];
        f32 lowCorner[3];
        f32 marginLow[3];
        f32 unitMarginHigh[3];
        f32 highCorner[3];
        f32 marginHigh[3];
        Vec3_Zero(low);
        Vec3_Zero(high);
        unitMarginLow[0] = kFloat_1_GolfPlayer;
        unitMarginLow[1] = kFloat_0_GolfPlayer;
        unitMarginLow[2] = kFloat_1_GolfPlayer;
        __ml__FfRC7WVector(marginLow, unitMarginLow, kFloat_16_0);
        __mi__FRC7WVectorRC7WVector(lowCorner, GolfBall__Fv(), marginLow);
        *(WVector*)low = *(WVector*)lowCorner;
        unitMarginHigh[0] = kFloat_1_GolfPlayer;
        unitMarginHigh[1] = kFloat_0_GolfPlayer;
        unitMarginHigh[2] = kFloat_1_GolfPlayer;
        __ml__FfRC7WVector(marginHigh, unitMarginHigh, kFloat_16_0);
        __pl__FRC7WVectorRC7WVector(highCorner, GolfBall__Fv(), marginHigh);
        *(WVector*)high = *(WVector*)highCorner;
        f32 bounds[6];
        vectorFieldAt(bounds, 0) = vectorFieldAt(low, 0);
        vectorFieldAt(bounds, 12) = vectorFieldAt(high, 0);
        ((CQuadTree*)CQuadTree_GetInstance())->GetObjList(bounds);
        (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, currentPlayerIndex()))->turnedThisFrame) = 0;
        (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, currentPlayerIndex()))->turnedLastFrame) = 0;
        f32 temporary[12];
        double angle = AsCGolfDoc(currentGolfDocBytes())->headingRadians;
        ((PlayerPoseState*)playerPoseBytes(this, currentPlayerIndex()))->headingRadians = (f32)angle;
        Matrix_SetAxisRotation(temporary, (f32)angle, 1);
        Matrix_CopyAffine(matrix, temporary);
        CGolfPlayer::UpdateSwingContactData(this, -1);
        (*(char**)&trailStartBoneEntry) = (char*)WPuppet_FindNamedBoneEntry(
            PlayerModel_GetPuppet(playerModelBytes(this, currentPlayerIndex())), golfPlayerStrings + kTextClubNear);
        (*(char**)&trailEndBoneEntry) = (char*)WPuppet_FindNamedBoneEntry(
            PlayerModel_GetPuppet(playerModelBytes(this, currentPlayerIndex())), golfPlayerStrings + kTextClubFar);
        ((CTailEffect*)((*(char**)&tailEffects[0])))->ClearTrail();
        ((CTailEffect*)((*(char**)&tailEffects[1])))->ClearTrail();
        ((CTailEffect*)((*(char**)&tailEffects[2])))->ClearTrail();
        ((CTailEffect*)((*(char**)&tailEffects[3])))->ClearTrail();
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextChar, 1);
        bindPetEffect(((u32)g_shadowManager), golfPlayerStrings + kTextChar,
                      playerModelBytes(this, currentPlayerIndex()));
        char* player = (char*)GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), currentPlayerIndex());
        char* hole = (*(char**)&AsCGolfDoc(currentGolfDocBytes())->holeRecords) +
                     AsCGolfDoc(currentGolfDocBytes())->currentHoleIndex * 32;
        f32 hp[3];
        f32 dist[3];
        vectorFieldAt(hp, 0) = vectorFieldAt(hole, 0xc);
        __mi__FRC7WVectorRC7WVector(dist, hp, player + 0x1c);
        dist[1] = kFloat_0_GolfPlayer;
        distanceToHole = (*(WVector*)dist).Magnitude();
        void* animations = ClubModel_FindCachedForItem(
            InstanceTable_Get(),
            (void*)(unsignedFieldAt(((u32)g_sharedDoc),
                                    currentPlayerIndex() * (int)sizeof(PlayerEquipmentSlotView) +
                                        (int)DECOMP_OFFSETOF(PlayerEquipmentSlotView, clubItemId))));
        (*(unsigned int*)&clubPuppet) =
            unsignedFieldAt(animations, 4 + ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category * 4);
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 1);
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, 1);
        ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv((void*)(((u32)g_shadowManager)),
                                                              golfPlayerStrings + kTextClub, (*(char**)&clubPuppet));
        singlePowerShotReady = 0;
        doublePowerShotDisabled = 0;
        break;
    }
    // IN argument=borrowed name, or null for ready name. Copies into the fixed pending-name buffer.
    case kPlayerQueueAnimation:
        if (cmd->argument)
            strcpy(playerAnimationBytes(this, currentPlayerIndex()) + 0x39, (const char*)cmd->argument);
        else {
            char* manager = currentGolfDocBytes();
            char* name = String_FormatTemporary(golfPlayerStrings + kTextReadyFormat, shotAnimationPrefix);
            strcpy(playerAnimationBytes(this, AsCGolfDoc(manager)->currentPlayer) + 0x39, name);
        }
        break;
    // IN argument2=borrowed name; OUT argument=void** borrowed bone, owned by the player model.
    case kPlayerFindBone: {
        void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* resource = (char*)playerModels + AsCGolfDoc(manager)->currentPlayer * kPlayerModelStride;
        *(void**)cmd->argument = WPuppet_FindOrCacheBone(PlayerModel_GetPuppet(resource), cmd->argument2);
        break;
    }
    // OUT argument=float* radians, clamped to the configured slope limits.
    case kPlayerGetGroundSlope: {
        void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        f32 value = *(f32*)((char*)playerStates + AsCGolfDoc(manager)->currentPlayer * kPlayerPoseStride + 0x58);
        f32 result;
        if (value < kMinimumGroundSlopeRadians_GolfPlayer)
            result = kMinimumGroundSlopeRadians_GolfPlayer;
        else if (value > kMaximumGroundSlopeRadians_GolfPlayer)
            result = kMaximumGroundSlopeRadians_GolfPlayer;
        else
            result = value;
        *(f32*)cmd->argument = result;
        break;
    }
    // IN argument=mode VALUE. Nonzero enters power-ready; zero restores ordinary ready/Screen state.
    case kPlayerSetPowerMode: {
        f32 enabledScreenValue;
        f32 disabledScreenValue;
        handlePlayerSetPowerMode(this, cmd, enabledScreenValue, disabledScreenValue);
        break;
    }
    // IN argument=Vec3*. Probe eight headings; store COUNT OF USABLE directions in ball +AC.
    case kPlayerTestSwingDirections: {
        u8 count = 0;
        f32 angle = kFloat_0_GolfPlayer;
        void* position = cmd->argument;
        f32 zero = angle;
        f32 increment = kSwingDirectionStepRadians_GolfPlayer;
        for (s32 i = 0; i < 8; ++i) {
            f32 delta[3];
            f32 base[3];

            base[1] = floatFieldAt(GolfBall__Fv(), 0xc0);
            f32 savedZero = zero;
            base[0] = savedZero;
            base[2] = zero;
            __mi__FRC7WVectorRC7WVector(delta, position, base);
            // Legacy collision name has affirmative CLEAR/USABLE polarity.
            if (CGolfPlayer::IsSwingPlacementUsable(this, delta, angle))
                ++count;
            __typeof__(angle + increment) _p553 = increment + angle;
            f32 savedIncrement = increment;
            __typeof__(_p553) _p531 = angle + savedIncrement;
            angle = _p531;
        }
        byteFieldAt(GolfBall__Fv(), 0xac) = count;
        break;
    }
    // OUT argument=float* elapsed seconds to *shot; unchanged if no cached swing-name match.
    case kPlayerGetShotKeyTime:
        selectName(this, 0);
        for (u32 i = 0; i < 16; ++i) {
            if (strcmp(s_swingAnimationNames[i], swingAnimationName) == 0) {
                *(f32*)cmd->argument =
                    floatFieldAt(playerAnimationBytes(this, currentPlayerIndex()), 0x64 + (((i * 4))));
                break;
            }
        }
        break;
    // No payload. Resolve swing name; queue it after the last second of the ready range when available.
    case kPlayerPrepareSwing: {
        selectName(this, 1);
        void* anim;
        if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode != 0) {
            const char* name;
            if (((ShotInstanceView*)InstanceTable_Get())->powerShotMode == 1 ||
                ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category != 0)
                name = String_FormatTemporary(golfPlayerStrings + kTextPowerReadyEndFormat, shotAnimationPrefix);
            else
                name = String_FormatTemporary(golfPlayerStrings + kTextDoublePowerReadyEndFormat, shotAnimationPrefix);
            anim = (void*)ObjectList_FindAnimationRangeByName(playerModelBytes(this, currentPlayerIndex()), name, 1, 0);
        } else {
            char* manager = currentGolfDocBytes();
            const char* name = String_FormatTemporary(golfPlayerStrings + kTextReadyFormat, shotAnimationPrefix);
            anim = (void*)ObjectList_FindAnimationRangeByName(
                playerModelBytes(this, AsCGolfDoc(manager)->currentPlayer), name, 1, 0);
        }
        if (floatFieldAt(anim, 4) - floatFieldAt(anim, 0) > kFloat_1_GolfPlayer) {
            animationCompletionNotifyPending = 1;
            CGolfPlayer_SetPlayerAnimation(this, pointerFieldAt(anim, 0x18),
                                           (floatFieldAt(anim, 4) - floatFieldAt(anim, 0)) - kFloat_1_GolfPlayer, -1);
            strcpy(playerAnimationBytes(this, currentPlayerIndex()) + 0x39, swingAnimationName);
        } else {
            animationCompletionNotifyPending = 0;

            if (!negligible((absoluteValue(floatFieldAt(anim, 4) - floatFieldAt(anim, 0)))))
                MessageBox_Report(0, golfPlayerStrings + kTextInvalidReadyAnimationWarning,
                                  golfPlayerStrings + kTextWarningTitle, 0);
        }
        break;
    }
    // IN argument=IFF item-ID VALUE. Toggle auxiliary selection or validate/apply parts, then rebuild effects.
    case kPlayerChangeEquippedItem: {
        u32 item = (u32)cmd->argument;
        if (item >> 26 == 0x1c) {
            char* slot = (char*)((u32)g_sharedDoc) + currentPlayerIndex() * 0x1d4 + ((item >> 19) & 0x7c);
            if (item == unsignedFieldAt(slot, 0x190))
                unsignedFieldAt(slot, 0x190) = 0;
            else
                unsignedFieldAt(slot, 0x190) = item;
            ((CGolfDoc*)currentGolfDocBytes())->RefreshPlayerSnapshotStats(currentPlayerIndex());
            sendPlayerEvent(this, 0x16b, 5, 0);
            RebuildAttachedEffects(currentPlayerIndex(), (void*)(1), (void*)(1));
        } else {
            void* player = ((CGolfDoc*)currentGolfDocBytes())->GetItemSelection(currentPlayerIndex());
            if (!ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, item))
                break;
            s32 changed =
                AvatarModel_ApplyValidatedItemChanges(playerModelBytes(this, currentPlayerIndex()), player, item);
            void* service = pointerFieldAt(g_resourceManager, 0);
            u32 result;
            ((void (**)(void*, s32, s32, void*))pointerFieldAt(service, 0))[9](service, 0x1f, 2, &result);
            if (changed) {
                UpdatePuppetBoundsForAnimationList(playerModelBytes(this, currentPlayerIndex()));
                ((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->flags |=
                    (kAnimationTransformDirty | kAnimationPlaying | kAnimationBodyLightDirty);
            }
            RebuildAttachedEffects(currentPlayerIndex(), (void*)(1), (void*)(1));
        }
        break;
    }
    // No payload. Enable current pose and its scene/club entries; animation visibility is a separate word.
    case kPlayerShow:
        (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, currentPlayerIndex()))->enabled) = 1;
        setPlayerUI(((u32)g_sceneManager), currentPlayerIndex(), 1);
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 1);
        break;
    // No payload. Disable current pose and scene/club entries.
    case kPlayerHide:
        (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, currentPlayerIndex()))->enabled) = 0;
        setPlayerUI(((u32)g_sceneManager), currentPlayerIndex(), 0);
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 0);
        break;
    // IN argument=borrowed hierarchy context. Each eligible iteration targets the CURRENT model, not loop i.
    case kPlayerSetChildState:
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i) {
            if (intFieldAt(GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), (u8)i), 8) != 3)
                WPuppet_SetHierarchyContextRecursive(
                    PlayerModel_GetPuppet(playerModelBytes(this, currentPlayerIndex())), cmd->argument);
        }
        break;
    // IN argument=update-only VALUE. Nonzero changes rank animations; zero also grounds/rebinds models.
    case kPlayerPlaceResults: {
        // a1 != 0 changes rank animation only. Full placement grounds named
        // rank spawns, samples initial events and creates a shadow per player.
        s32 updateOnly = (s32)cmd->argument;
        GfxObjList_RemoveGroupByName(((u32)g_shadowManager), golfPlayerStrings + kTextChar);
        GfxObjList_RemoveGroupByName(((u32)g_shadowManager), golfPlayerStrings + kTextCaddie);
        GfxObjList_RemoveGroupByName(((u32)g_shadowManager), golfPlayerStrings + kTextClub);
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i) {
            if (AsCSharedDoc(g_sharedDoc)->golfRuleType != 1 &&
                intFieldAt(GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), (u8)i), 8) == 3)
                continue;
            if (updateOnly) {
                SelectRankAnimation((unsigned int)(i));
                continue;
            }
            u32 players = AsCGolfDoc(currentGolfDocBytes())->playerCount;
            u32 rank = ((u32)byteFieldAt(GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), (u8)i), 0xc4) >> 4) & 15;
            if (1 > rank)
                rank = 1;
            else if (rank > players)
                rank = players;
            const char* name =
                String_FormatTemporary(golfPlayerStrings + kTextRankedSpawnFormat,
                                       (u8)((CGolfDoc*)currentGolfDocBytes())->GetActivePlayerCount(), (u8)rank);
            void* spawn = CourseData_FindNamedPoint((*(char**)&AsCGolfDoc(currentGolfDocBytes())->courseData), name);
            WPuppet_ShowBoneSelection(PlayerModel_GetPuppet(playerModelBytes(this, i)), 0);
            WPuppet_HideBoneSelection(PlayerModel_GetPuppet(playerModelBytes(this, i)),
                                      golfPlayerStrings + kTextBone01);
            PuppetOwner_SetOpacityRecursive(playerModelBytes(this, i), 255);
            setPlayerUI(((u32)g_sceneManager), i, 1);
            Matrix_SetIdentityAffine(playerAnimationBytes(this, i) + 8);
            f32 out[3];
            f32 pos[3];
            vectorFieldAt(pos, 0) = vectorFieldAt(spawn, 0x24);
            GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), pos, 0, 0, 0);
            *(WVector*)(playerAnimationBytes(this, i) + 0x2c) = *(WVector*)out;

            ((PlayerAnimationRecord*)playerAnimationBytes(this, i))->visible = 1;
            SelectRankAnimation((unsigned int)(i));
            void* puppet = ObjectList_GetSelected(playerModelBytes(this, i));
            WPuppet_SetAnimationTimeFromRanges(puppet, 0, ((PlayerAnimationRecord*)playerAnimationBytes(this, i))->time,
                                               0, 0);
            Model_SetMatrixRecursive(PlayerModel_GetPuppet(playerModelBytes(this, i)), puppet,
                                     playerAnimationBytes(this, i) + 8);
            refreshPlayerLighting(this, -1);
            char effect[16];
            sprintf(effect, golfPlayerStrings + kTextCharD, i);
            void* settings = effectSettings((commandSupportStorage + 0xc));
            GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
                (void*)(((u32)g_shadowManager)), effect, kFloat_0_05_PlayerShadow, 16, golfPlayerStrings + kTextSun,
                AsRenderPacketFields(settings)->auxiliaryState);
            bindPetEffect(((u32)g_shadowManager), effect, playerModelBytes(this, i));
            ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), effect, 1);
            PlayerModel_SetDefaultPuppetLayerEnabled(playerModelBytes(this, i), 0);
            void* animation = (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, i))->animationObject);
            f32 start = floatFieldAt(animation, 0);
            AnimationEventParser_ProcessFrameRange((char*)&sub_vtbl, ObjectList_GetSelected(playerModelBytes(this, i)),
                                                   start, start, animation, i, 1);
        }
        DrawScene_SetNamedObjectPuppet((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 0);
        break;
    }
    // No payload. Remove only the per-player charN result-shadow groups.
    case kPlayerRemoveResultShadows: {
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i) {
            char effect[16];
            sprintf(effect, golfPlayerStrings + kTextCharD, i);
            GfxObjList_RemoveGroupByName(((u32)g_shadowManager), effect);
        }
        break;
    }
    // No payload. Create char/club shadow entries from the render defaults.
    case kPlayerCreateShotShadows: {
        void* settings = effectSettings((commandSupportStorage + 0xc));
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            (void*)(((u32)g_shadowManager)), golfPlayerStrings + kTextChar, kFloat_0_05_PlayerShadow, 16,
            golfPlayerStrings + kTextSun, AsRenderPacketFields(settings)->auxiliaryState);
        settings = effectSettings((commandSupportStorage + 0xc));
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            (void*)(((u32)g_shadowManager)), golfPlayerStrings + kTextClub, kFloat_0_05_PlayerShadow, 256,
            golfPlayerStrings + kTextSun, AsRenderPacketFields(settings)->auxiliaryState);
        break;
    }
    // IN argument=club-set ID VALUE; argument2=recache flag. Commit selection only if creation succeeds.
    case kPlayerSelectClub: {
        handlePlayerSelectClub(this, cmd);
        break;
    }
    // No payload. Reload slot zero from shared outfit, retry fallback resources, then refresh bounds/bindings.
    case kPlayerReloadPreview: {
        void* player = ((CGolfDoc*)currentGolfDocBytes())->GetItemSelection(0);
        u32 global = ((u32)g_sharedDoc);
        u8 mode = PlayerRank_FindTierForValue((void*)(unsignedFieldAt(global, 0x44)));
        u32 flags = byteFieldAt(global, 0x188);
        ItemSelection_BuildCharacterOutfit(player, unsignedFieldAt(global, 0x120), (flags >> 3) & 31, flags & 7,
                                           ((SharedPlayerSelectionView*)global)->outfit.partSelections, mode);
        s32 kind = 2;
        if (unsignedFieldAt(((u32)g_sharedDoc), 0x1dc))
            kind = 1;
        if (!AvatarModel_LoadCharacterResources((*(char**)&playerModels), player, (char*)((u32)g_sharedDoc) + 0x190, 1,
                                                kind, 1, -1, 0)) {
            ItemSelection_RefreshPrimaryItems(player);
            AvatarModel_LoadCharacterResources((*(char**)&playerModels), player, (char*)((u32)g_sharedDoc) + 0x190, 1,
                                               kind, 0, -1, 0);
        }
        UpdatePuppetBoundsForAnimationList((*(char**)&playerModels));
        u32 ui = ((u32)g_sceneManager);
        void* model = PlayerModel_GetPuppet((*(char**)&playerModels));
        DrawScene_SetNamedObjectPuppet((void*)(ui), golfPlayerStrings + kTextPLAYER0, (void*)((u32)model));
        bindPetEffect(((u32)g_shadowManager), golfPlayerStrings + kTextChar, (*(char**)&playerModels));
        CacheSwingPlacements(0);
        break;
    }
    // OUT argument=u32* current animation mode (PlayerAnimationMode).
    case kPlayerGetAnimationMode: {
        void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        *(u32*)cmd->argument =
            *(u32*)((char*)animationStates + AsCGolfDoc(manager)->currentPlayer * kAnimationStateStride + 0x60);
        break;
    }
    // IN argument=Vec3* world point, argument2=borrowed range name. Ground/sample active slot; hide other slot.
    case kPlayerPlacePreview: {
        // Ground the supplied world point, sample the named range, bind club/
        // shadow, hide the other slot, refresh lighting and rebuild item effects.
        // hasActivePlayerType() also writes a cached task pointer; repeated calls
        // below are preserved, including the legacy 255 fallback slot value.
        WPuppet_ShowBoneSelection(
            PlayerModel_GetPuppet(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255))), 0);
        WPuppet_HideBoneSelection(
            PlayerModel_GetPuppet(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255))),
            golfPlayerStrings + kTextBone01);
        PuppetOwner_SetOpacityRecursive(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)),
                                        255);
        Matrix_SetIdentityAffine(playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)) + 8);
        s32 index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        f32 out[3];
        f32 pos[3];
        vectorFieldAt(pos, 0) = vectorFieldAt(cmd->argument, 0);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), pos, 0, 0, 0);
        *(WVector*)(playerAnimationBytes(this, index) + 0x2c) = *(WVector*)out;
        (*(u32*)&(
              (PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
              ->mode) = 0;
        (*(u8*)&(
              (PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
              ->pendingAnimationName[0]) = 0;
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        void* anim = (void*)ObjectList_FindAnimationRangeByName(
            playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)), (const char*)cmd->argument2,
            1, 0);
        (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, index))->animationObject) = (char*)anim;
        if ((*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(
                            this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                  ->animationObject)) {
            index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
            ((PlayerAnimationRecord*)playerAnimationBytes(this, index))->time =
                floatFieldAt((*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(
                                             this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                                   ->animationObject),
                             0);
        } else
            ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                ->time = kFloat_0_GolfPlayer;
        ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
            ->visible = 1;
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        WPuppet_SetAnimationTimeFromRanges(ObjectList_GetSelected(playerModelBytes(this, index)), 0,
                                           ((PlayerAnimationRecord*)playerAnimationBytes(this, otherIndex))->time, 0,
                                           0);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 infoIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        void* puppet = ObjectList_GetSelected(playerModelBytes(this, otherIndex));
        Model_SetMatrixRecursive(PlayerModel_GetPuppet(playerModelBytes(this, index)), puppet,
                                 playerAnimationBytes(this, infoIndex) + 8);
        ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
            ->flags |= (kAnimationAttachmentDirty | kAnimationSamplePose);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        setPlayerUI(((u32)g_sceneManager), index, 1);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        void* animations = ClubModel_FindCachedForItem(
            InstanceTable_Get(), (void*)(unsignedFieldAt(((u32)g_sharedDoc), index * 0x1d4 + 0x1cc)));
        void* selected =
            pointerFieldAt(animations, 4 + ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category * 4);
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 1);
        DrawScene_SetNamedObjectPuppet((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor,
                                       (void*)((u32)selected));
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, 1);
        ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv((void*)(((u32)g_shadowManager)),
                                                              golfPlayerStrings + kTextClub, selected);
        (*(char**)&clubPuppet) = (char*)selected;
        PlayerModel_SetDefaultPuppetLayerEnabled(
            playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)), 0);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        infoIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 petIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        char* records = (*(char**)&animationStates) + 4;
        f32 start;
        start = floatFieldAt(*(void**)(records + otherIndex * 0xb4), 0);
        f32 end = floatFieldAt(*(void**)(records + infoIndex * 0xb4), 0);
        void* animation = *(void**)(records + index * 0xb4);
        AnimationEventParser_ProcessFrameRange((char*)&sub_vtbl,
                                               ObjectList_GetSelected(playerModelBytes(this, petIndex)), start, end,
                                               animation, petIndex, 1);
        ((PlayerAnimationRecord*)playerAnimationBytes(this, 1 - (hasActivePlayerType() ? currentPlayerIndex() : 255)))
            ->visible = 0;
        setPlayerUI(((u32)g_sceneManager), 1 - (hasActivePlayerType() ? currentPlayerIndex() : 255), 0);
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextChar, 1);
        bindPetEffect(((u32)g_shadowManager), golfPlayerStrings + kTextChar,
                      playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)));
        *(WVector*)((char*)&lightDirection) = *(WVector*)(4 + s_sceneLighting);
        (*(unsigned int*)&ambientLightColor) =
            Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x14), kFloat_1_GolfPlayer);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        if (index < 0)
            index = currentPlayerIndex();
        if (index >= 0) {
            if (4 > index) {
                void* part = WPuppet_FindBoneByName(PlayerModel_GetPuppet(playerModelBytes(this, index)),
                                                    golfPlayerStrings + kTextHead);
                if (part) {
                    float gain = ((CQuadTree*)CQuadTree_GetInstance())
                                     ->DefaultLightingFactor(&((PuppetBoneView*)part)->worldMatrix.position);
                    (*(unsigned int*)&lightColor) = Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x10), gain);
                }
            }
        }
        RebuildAttachedEffects((hasActivePlayerType() ? currentPlayerIndex() : 255), (void*)(1), (void*)(1));
        break;
    }
    // IN argument=Vec3* world point AND club-presence test; argument2=borrowed range name. Point must be readable.
    case kPlayerPlaceTurnPlayer: {
        // Similar to preview placement, but exposes only the selected turn slot
        // and derives the club visibility request from the non-null a1 payload.
        (hasActivePlayerType() ? currentPlayerIndex() : 255);
        bool visible = (0 != cmd->argument);
        WPuppet_ShowBoneSelection(
            PlayerModel_GetPuppet(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255))), 0);
        WPuppet_HideBoneSelection(
            PlayerModel_GetPuppet(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255))),
            golfPlayerStrings + kTextBone01);
        PuppetOwner_SetOpacityRecursive(playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)),
                                        255);
        Matrix_SetIdentityAffine(playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)) + 8);
        f32 out[3];
        s32 index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        f32 pos[3];
        vectorFieldAt(pos, 0) = vectorFieldAt(cmd->argument, 0);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), pos, 0, 0, 0);
        *(WVector*)(playerAnimationBytes(this, index) + 0x2c) = *(WVector*)out;
        (*(u32*)&(
              (PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
              ->mode) = 0;
        (*(u8*)&(
              (PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
              ->pendingAnimationName[0]) = 0;
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        void* anim = (void*)ObjectList_FindAnimationRangeByName(
            playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)), (const char*)cmd->argument2,
            1, 0);
        (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, index))->animationObject) = (char*)anim;
        if ((*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(
                            this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                  ->animationObject)) {
            index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
            ((PlayerAnimationRecord*)playerAnimationBytes(this, index))->time =
                floatFieldAt((*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(
                                             this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                                   ->animationObject),
                             0);
        } else
            ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
                ->time = kFloat_0_GolfPlayer;
        ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
            ->visible = 1;
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        WPuppet_SetAnimationTimeFromRanges(ObjectList_GetSelected(playerModelBytes(this, index)), 0,
                                           ((PlayerAnimationRecord*)playerAnimationBytes(this, otherIndex))->time, 0,
                                           0);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 infoIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        void* puppet = ObjectList_GetSelected(playerModelBytes(this, otherIndex));
        Model_SetMatrixRecursive(PlayerModel_GetPuppet(playerModelBytes(this, index)), puppet,
                                 playerAnimationBytes(this, infoIndex) + 8);
        ((PlayerAnimationRecord*)playerAnimationBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)))
            ->flags |= (kAnimationAttachmentDirty | kAnimationSamplePose);
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i) {
            *(signed char*)(playerAnimationBytes(this, i) + 0x38) =
                (hasActivePlayerType() ? currentPlayerIndex() : 255) == i;
            index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
            setPlayerUI(((u32)g_sceneManager), i, (i == index));
        }
        DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, visible);
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, visible);
        if (visible) {
            index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
            void* animations = ClubModel_FindCachedForItem(
                InstanceTable_Get(), (void*)(unsignedFieldAt(((u32)g_sharedDoc), index * 0x1d4 + 0x1cc)));
            void* selected = 0;
            if (animations) {
                selected = pointerFieldAt(animations,
                                          4 + ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category * 4);
                if (selected) {
                    DrawScene_SetNamedObjectPuppet((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor,
                                                   (void*)((u32)selected));
                    ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv((void*)(((u32)g_shadowManager)),
                                                                          golfPlayerStrings + kTextClub, selected);
                    (*(char**)&clubPuppet) = (char*)selected;
                }
            }
            if (animations == 0 || !selected)
                (*(unsigned int*)&clubPuppet) = 0;
        } else
            (*(unsigned int*)&clubPuppet) = 0;
        PlayerModel_SetDefaultPuppetLayerEnabled(
            playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)), 0);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        infoIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        otherIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        s32 petIndex = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        char* records = (*(char**)&animationStates) + 4;
        s32 savedInfoIndex = infoIndex;
        f32 start;
        f32 end = floatFieldAt(*(void**)(records + savedInfoIndex * 0xb4), 0);
        start = floatFieldAt(*(void**)(records + otherIndex * 0xb4), 0);
        void* animation = *(void**)(records + index * 0xb4);
        f32 savedStart = start;
        f32 savedEnd = end;
        AnimationEventParser_ProcessFrameRange((char*)&sub_vtbl,
                                               ObjectList_GetSelected(playerModelBytes(this, petIndex)), savedStart,
                                               savedEnd, animation, petIndex, 1);
        ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextChar, 1);
        bindPetEffect(((u32)g_shadowManager), golfPlayerStrings + kTextChar,
                      playerModelBytes(this, (hasActivePlayerType() ? currentPlayerIndex() : 255)));
        *(WVector*)((char*)&lightDirection) = *(WVector*)(s_sceneLighting + 4);
        (*(unsigned int*)&ambientLightColor) =
            Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x14), kFloat_1_GolfPlayer);
        index = (hasActivePlayerType() ? currentPlayerIndex() : 255);
        index = (0 <= index) ? index : currentPlayerIndex();
        if (0 <= index) {
            if (4 > index) {
                s32 savedIndex = index;
                void* part = WPuppet_FindBoneByName(PlayerModel_GetPuppet(playerModelBytes(this, savedIndex)),
                                                    golfPlayerStrings + kTextHead);
                if (part) {
                    float gain = ((CQuadTree*)CQuadTree_GetInstance())
                                     ->DefaultLightingFactor(&((PuppetBoneView*)part)->worldMatrix.position);
                    (*(unsigned int*)&lightColor) = Color_ScaleRGB(unsignedFieldAt(s_sceneLighting, 0x10), gain);
                }
            }
        }
        RebuildAttachedEffects((hasActivePlayerType() ? currentPlayerIndex() : 255), (void*)(1), (void*)(1));
        break;
    }
    // IN argument/argument2=names; argument3=Vec3* world target. Only pass zero runs, so second name is unused.
    case kPlayerPlaceWinner: {
        const char* names[2] = {(const char*)cmd->argument, (const char*)cmd->argument2};
        f32 target[3];
        *(WVector*)target = *(WVector*)((void*)cmd->argument3);
        f32 zero = kFloat_0_GolfPlayer;
        s32 index = 0;
        for (s32 pass = 0; pass < 2; ++pass) {
            f32 world[3];
            f32 angles[3];
            if (pass == 1)
                break;
            index = ((CGolfDoc*)currentGolfDocBytes())->ResolveMatchTurnFromHoleStrokes();
            char* pet = playerModelBytes(this, index);
            char* info = playerAnimationBytes(this, index);
            WPuppet_ShowBoneSelection(PlayerModel_GetPuppet(pet), 0);
            WPuppet_HideBoneSelection(PlayerModel_GetPuppet(pet), golfPlayerStrings + kTextBone01);
            PuppetOwner_SetOpacityRecursive(pet, 255);
            bool special = specialHole();
            double angle = special ? kFloat_0_GolfPlayer : kWinnerHeadingRadians_GolfPlayer;
            Matrix_SetIdentityAffine(info + 8);
            angles[0] = zero;
            angles[1] = angle;
            angles[2] = zero;
            Matrix_AppendEulerRotation(((info + 8)), angles);
            s32 rotationIndex = index;
            if (0 > rotationIndex)
                rotationIndex = currentPlayerIndex();
            s32 resolved = rotationIndex;
            if (0 > resolved)
                resolved = currentPlayerIndex();
            void* matrix = playerPoseBytes(this, resolved) + 0x10;
            resolved = rotationIndex;
            if (resolved < 0)
                resolved = currentPlayerIndex();
            char* rotation = playerPoseBytes(this, resolved);
            resolved = rotationIndex;
            if (0 > resolved)
                resolved = currentPlayerIndex();
            (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, resolved))->turnedThisFrame) = 0;
            (*(u32*)&((PlayerPoseState*)playerPoseBytes(this, resolved))->turnedLastFrame) = 0;
            ((PlayerPoseState*)rotation)->headingRadians = angle;
            f32 temporary[12];
            Matrix_SetAxisRotation(temporary, (f32)angle, 1);
            Matrix_CopyAffine(matrix, temporary);

            CGolfPlayer::UpdateSwingContactData(this, rotationIndex);
            ((PlayerAnimationRecord*)playerAnimationBytes(this, rotationIndex))->flags |=
                (kAnimationAttachmentDirty | kAnimationSamplePose | kAnimationPositionDirty);
            f32 pos[3];
            f32 out[3];
            f32 offset[3];
            Matrix_TransformPoint(offset, (char*)&defaultByPlayer[index], (((info + 8))));
            __pl__FRC7WVectorRC7WVector(world, target, offset);
            vectorFieldAt(pos, 0) = vectorFieldAt(world, 0);
            GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), pos, 0, 0, 0);
            *(WVector*)(info + 0x2c) = *(WVector*)out;
            unsignedFieldAt(info, 0x60) = 0;
            byteFieldAt(info, 0x39) = 0;
            void* anim = (void*)ObjectList_FindAnimationRangeByName(pet, names[pass], 1, 0);
            pointerFieldAt(info, 4) = (char*)anim;
            if (anim)
                floatFieldAt(info, 0) = floatFieldAt(anim, 0);
            else
                floatFieldAt(info, 0) = zero;
            byteFieldAt(info, 0x38) = 1;
            void* puppet = ObjectList_GetSelected(pet);
            WPuppet_SetAnimationTimeFromRanges(puppet, 0, floatFieldAt(info, 0), 0, 0);
            Model_SetMatrixRecursive(PlayerModel_GetPuppet(pet), puppet, info + 8);
            ((PlayerAnimationRecord*)playerAnimationBytes(this, index))->flags |=
                (kAnimationAttachmentDirty | kAnimationSamplePose);
            setPlayerUI(((u32)g_sceneManager), index, 1);
            void* settings = effectSettings((commandSupportStorage + 0xc));
            GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
                (void*)(((u32)g_shadowManager)), golfPlayerStrings + kTextChar, kFloat_0_05_PlayerShadow, 16,
                golfPlayerStrings + kTextSun, AsRenderPacketFields(settings)->auxiliaryState);
            bindPetEffect(((u32)g_shadowManager), golfPlayerStrings + kTextChar, playerModelBytes(this, index));
            void* animations = ClubModel_FindCachedForItem(
                InstanceTable_Get(), (void*)(unsignedFieldAt(((u32)g_sharedDoc), index * 0x1d4 + 0x1cc)));
            if (!animations)
                continue;
            void* selected;
            if (index == currentPlayerIndex())
                selected = pointerFieldAt(animations,
                                          4 + ((ShotInstanceView*)InstanceTable_Get())->selectedClub->category * 4);
            else
                selected = pointerFieldAt(
                    animations,
                    4 + byteFieldAt(GolfPlayer_GetEntryByIndex(currentGolfDocBytes(), (u8)index), 0xbd) * 4);
            if (!selected)
                continue;
            DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 1);
            DrawScene_SetNamedObjectPuppet((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor,
                                           (void*)((u32)selected));
            ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, 1);
            ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv((void*)(((u32)g_shadowManager)),
                                                                  golfPlayerStrings + kTextClub, selected);
            (*(char**)&clubPuppet) = (char*)selected;
            PlayerModel_SetDefaultPuppetLayerEnabled(playerModelBytes(this, index), 0);
            void* animation = (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, index))->animationObject);
            f32 start = floatFieldAt(animation, 0);
            AnimationEventParser_ProcessFrameRange((char*)&sub_vtbl,
                                                   ObjectList_GetSelected(playerModelBytes(this, index)), start, start,
                                                   animation, index, 1);
        }
        for (s32 i = 0; i < AsCGolfDoc(currentGolfDocBytes())->playerCount; ++i) {
            if (i != index) {
                ((PlayerAnimationRecord*)playerAnimationBytes(this, i))->visible = 0;
                setPlayerUI(((u32)g_sceneManager), i, 0);
            }
        }
        refreshPlayerLighting(this, index);
        RebuildAttachedEffects(index, (void*)(1), (void*)(1));
        break;
    }
    // No payload. Copy the club/power-specific follow-through name into the pending-name buffer.
    case kPlayerQueueFollowThrough: {
        handlePlayerQueueFollowThrough(this, cmd);
        break;
    }
    // IN argument=boolean VALUE. Update both hidden flag and scene/shadow visibility.
    case kPlayerSetClubVisible:
        if (cmd->argument == 0) {
            ((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->flags |= kAnimationClubHidden;
            DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 0);
            ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, 0);
        } else {
            ((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->flags &= ~kAnimationClubHidden;
            DrawScene_SetNamedObjectEnabled((void*)(((u32)g_sceneManager)), golfPlayerStrings + kTextClubActor, 1);
            ShadowModel_SetNamedProjectionEnabled(((u32)g_shadowManager), golfPlayerStrings + kTextClub, 1);
        }
        break;
    // No payload. Format every range name/index for diagnostics; formatting return value is discarded.
    case kPlayerListAnimations: {
        void* puppet = ObjectList_GetSelected(playerModelBytes(this, currentPlayerIndex()));
        if (puppet == 0)
            break;
        void* list = pointerFieldAt(puppet, 0x144);
        if (!list)
            break;
        char** item = (char**)pointerFieldAt(list, 0);
        s32 i = 0;
        while (item != (char**)pointerFieldAt(list, 0) + unsignedFieldAt(list, 4)) {
            String_FormatTemporary(golfPlayerStrings + kTextDS, i, pointerFieldAt(*item, 0x18));
            i = 1 + i;
            item++;
        }
        break;
    }
    // IN argument=zero-based range VALUE. Copy its descriptor to shared scratch; install scratch as active range.
    case kPlayerSelectAnimation: {
        s32 selected = (s32)cmd->argument;
        void* puppet = ObjectList_GetSelected(playerModelBytes(this, currentPlayerIndex()));
        if (puppet == 0)
            break;
        void* list = pointerFieldAt(puppet, 0x144);
        char** item = (char**)pointerFieldAt(list, 0);
        char** end = item + unsignedFieldAt(list, 4);
        for (s32 i = 0; item != end; ++i, ++item) {
            if (selected != i)
                continue;
            (*(u8*)&((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))
                  ->pendingAnimationName[0]) = 0;
            CGolfPlayer_SetPlayerAnimation(this, pointerFieldAt(*item, 0x18), kFloat_0_GolfPlayer, -1);
            char* anim =
                (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->animationObject);
            char* copy = (commandSupportStorage + 0x88);
            floatFieldAt(copy, 0) = floatFieldAt(anim, 0);
            floatFieldAt(copy, 4) = floatFieldAt(anim, 4);
            floatFieldAt(copy, 8) = floatFieldAt(anim, 8);
            unsignedFieldAt(copy, 0xc) = unsignedFieldAt(anim, 0xc);
            floatFieldAt(copy, 0x10) = floatFieldAt(anim, 0x10);
            unsignedFieldAt(copy, 0x14) = unsignedFieldAt(anim, 0x14);
            unsignedFieldAt(copy, 0x18) = unsignedFieldAt(anim, 0x18);
            unsignedFieldAt(copy, 0x1c) = unsignedFieldAt(anim, 0x1c);
            byteFieldAt(copy, 0x20) = byteFieldAt(anim, 0x20);
            unsignedFieldAt(copy, 0x24) = unsignedFieldAt(anim, 0x24);
            unsignedFieldAt(copy, 0x28) = unsignedFieldAt(anim, 0x28);
            *(unsigned short*)(copy + 0x2c) = *(unsigned short*)(anim + 0x2c);
            *(unsigned short*)(copy + 0x2e) = *(unsigned short*)(anim + 0x2e);
            *(unsigned short*)(copy + 0x30) = *(unsigned short*)(anim + 0x30);
            byteFieldAt(copy, 0x32) = byteFieldAt(anim, 0x32);
            byteFieldAt(copy, 0x33) = byteFieldAt(anim, 0x33);
            sprintf((commandSupportStorage + 0xc0), golfPlayerStrings + kTextReadyFormat, shotAnimationPrefix);
            pointerFieldAt(copy, 0x1c) = (commandSupportStorage + 0xc0);
            (*(char**)&((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->animationObject) =
                copy;
            if (strstr(pointerFieldAt(*item, 0x18), golfPlayerStrings + kTextRankAnimationSuffix))
                ((PlayerAnimationRecord*)playerAnimationBytes(this, currentPlayerIndex()))->flags |=
                    kAnimationClubHidden;
            break;
        }
        break;
    }
    // OUT argument=Vec3* cached placement for resolved turn/selected club. Resolved turn must be valid.
    case kPlayerGetGripOffset: {
        s32 player = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))->ResolveTurnAfterMatchHoleOut();
        void* mode = ((ShotInstanceView*)InstanceTable_Get())->selectedClub;
        *(WVector*)cmd->argument = *(WVector*)&byPlayerAndClub[player][((SelectedClubView*)mode)->category];
        break;
    }
    // OUT argument=u32* contact word, argument2=float* companion scalar; precise semantics unresolved.
    case kPlayerGetSwingContactResult:
        *(u32*)cmd->argument = unk_2CD;
        *(f32*)cmd->argument2 = unk_2D0;
        break;
    // IN argument=value word copied verbatim to processingSuspended.
    case kPlayerSuspendProcessing:
        (*(unsigned int*)&processingSuspended) = (u32)cmd->argument;
        break;
    // IN argument=slot VALUE. Remove shadow reference, release mask 7, detach scene puppet.
    case kPlayerReleaseResources: {
        u32 shadowManagerHandle;
        s32 playerSlot;
        handlePlayerReleaseResources(this, cmd, shadowManagerHandle, playerSlot);
        break;
    }
    // IN argument=slot VALUE. Remove shadow reference, release mask 2, detach scene puppet.
    case kPlayerReleaseModel: {
        u32 shadowManagerHandle;
        s32 playerSlot;
        handlePlayerReleaseModel(this, cmd, shadowManagerHandle, playerSlot);
        break;
    }
    // IN argument=slot VALUE; argument2==1 selects kind 4. Begin only when no model/parts are loaded.
    case kPlayerBeginAvatarLoad: {
        handlePlayerBeginAvatarLoad(this, cmd);
        break;
    }
    // OUT argument=u32*. One means pending flag is clear; this does not poll the loader itself.
    case kPlayerIsAvatarLoadComplete:
        *(u32*)cmd->argument = (*(unsigned int*)&avatarPreloadPending) == 0;
        break;
    // No payload. Install pending avatar and release its loader through ApplyPendingAvatarLoad.
    case kPlayerApplyAvatarLoad:
        ApplyPendingAvatarLoad();
        break;
    // IN argument2=slot VALUE; OUT argument=int* model-and-parts predicate.
    case kPlayerHasModel:
        *(s32*)cmd->argument =
            ResourceModel801B_HasModelAndParts((char*)playerModels + (s32)cmd->argument2 * kPlayerModelStride);
        break;
    // No payload. Invoke the display-refresh virtual operation.
    case kPlayerRefreshDisplay:
        invokePlayerDisplayRefreshCallback(this);
        break;
    // No payload. Switch normal players to scripted slots.
    case kPlayerBeginCutsceneSlots:
        SetCutsceneSlotsEnabled(1);
        break;
    // No payload. Restore normal players from scripted slots.
    case kPlayerEndCutsceneSlots:
        SetCutsceneSlotsEnabled(0);
        break;
    // IN argument=load-list setup payload, forwarded unchanged; nested ownership is external.
    case kPlayerBuildPendingLoads:
        RebuildPendingModelLoads((int)cmd->argument);
        break;
    // OUT argument=u32* pending-load poll result.
    case kPlayerPollPendingLoads:
        *(u32*)cmd->argument = (unsigned int)PollPendingModelLoads();
        break;
    // IN argument=renderer option word; argument2=slot VALUE. Forward only when puppet exists.
    case kPlayerSetPuppetState: {
        handlePlayerSetPuppetState(this, cmd);
        break;
    }
    // No payload. Disable all four trails, then clear each allocated trail history.
    case kPlayerClearTrails:
        trailEnabled[0] = 0;
        trailEnabled[1] = 0;
        trailEnabled[2] = 0;
        trailEnabled[3] = 0;
        if (tailEffects[0])
            ((CTailEffect*)(tailEffects[0]))->ClearTrail();
        if (tailEffects[1])
            ((CTailEffect*)(tailEffects[1]))->ClearTrail();
        if (tailEffects[2])
            ((CTailEffect*)(tailEffects[2]))->ClearTrail();
        if (tailEffects[3])
            ((CTailEffect*)(tailEffects[3]))->ClearTrail();
        break;
    // OUT argument=u8* special-victory predicate.
    case kPlayerCheckSpecialVictory:
        *(u8*)cmd->argument = specialHole();
        break;
    // Recognized no-op messages: no payload is consumed and no state changes.
    case 8:
    case 0x13:
    case 0x14:
    case 0xd:
        break;
    default:
        break;
    }
}

#pragma pop

// Return the shot-gauge data beginning with shotPowerGaugeX in the current golf document.
void* GolfDoc_GetShotGaugeData() {
    return (char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x2C4;
}

// Choose the club/power/turning ready animation and refresh the active swing reach.
void CGolfPlayer::ResetReadyAnimation(void* playerObject) {
    char* self = (char*)playerObject;
    unsigned char clubCategory = *(unsigned char*)((char*)*(void**)((char*)InstanceTable_Get() + 4) + 0xA);
    switch (clubCategory) {
    case 0:
        strcpy(self + 0x65, golfPlayerStrings + kTextWoodShot);
        break;
    case 1:
        strcpy(self + 0x65, golfPlayerStrings + kTextIronShot);
        break;
    case 2:
        strcpy(self + 0x65, golfPlayerStrings + kTextSandWedgeShot);
        break;
    case 3:
        strcpy(self + 0x65, golfPlayerStrings + kTextPuttingShot);
        break;
    }

    char* entry = ((char*)AsCGolfPlayer(self)->playerStates) +
                  AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer * kPlayerPoseStride;
    char* readyAnimationName;
    if (*(int*)((char*)InstanceTable_Get() + kCommittedPowerModeOffset) != 0) {
        if (*(int*)((char*)InstanceTable_Get() + kCommittedPowerModeOffset) == 1 ||
            *(unsigned char*)((char*)*(void**)((char*)InstanceTable_Get() + 4) + 0xA) != 0) {
            if (!*(int*)(entry + 4) && !*(int*)(entry + 8))
                readyAnimationName = String_FormatTemporary(golfPlayerStrings + kTextPowerReadyEndFormat, self + 0x65);
            else
                readyAnimationName = String_FormatTemporary(golfPlayerStrings + kTextPowerSidestepFormat, self + 0x65);
        } else {
            if (!*(int*)(entry + 4) && !*(int*)(entry + 8))
                readyAnimationName =
                    String_FormatTemporary(golfPlayerStrings + kTextDoublePowerReadyEndFormat, self + 0x65);
            else
                readyAnimationName =
                    String_FormatTemporary(golfPlayerStrings + kTextDoublePowerSidestepFormat, self + 0x65);
        }
    } else {
        if (!*(int*)(entry + 4) && !*(int*)(entry + 8))
            readyAnimationName = String_FormatTemporary(golfPlayerStrings + kTextReadyFormat, self + 0x65);
        else
            readyAnimationName = String_FormatTemporary(golfPlayerStrings + kTextSidestepFormat, self + 0x65);
    }
    CGolfPlayer_SetPlayerAnimation((CGolfPlayer*)(self), readyAnimationName, kFloat_0_GolfPlayer, -1);
    char* view = (char*)InstanceTable_Get();
    unsigned char player = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
    float swingReach =
        *(float*)(self + player * 0x40 + *(unsigned char*)((char*)*(void**)(view + 4) + 0xA) * 0x10 + 0x128);
    *(float*)(self + 0x44) = swingReach;
    char* animationBytes = ((char*)AsCGolfPlayer(self)->animationStates);
    *(int*)(animationBytes +
            AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer * kAnimationStateStride + 0x60) =
        0;
}

#pragma dont_inline on
// Choose swing, impact, and terrain sounds from the animation, club, power, and special-shot state.
void CGolfPlayer::PlaySwingAndTerrainSounds(int index) {
    int soundStrength;
    int usesNoveltyClub;
    int player;
    if (index < 0 || index >= GolfDoc_GetPlayerCount())
        return;
    char* soundStrings = golfPlayerStrings;
    char* animationBytes = ((char*)animationStates) + index * kAnimationStateStride;
    if ((int)CString::FindSubstringForwarder(*(char**)(*(char**)(animationBytes + 4) + 0x18),
                                             soundStrings + kTextSwingSuffix)) {
        if (!InstanceTable_GetClubCategory(InstanceTable_Get())) {
            if (!(int)CString::FindSubstringForwarder((char*)&swingAnimationName, soundStrings + kTextEarlySwingSuffix))
                CGolfPlayer::PlaySwingSound(this, index, (void*)(soundStrings + kTextSwingVain));
            else
                CGolfPlayer::PlaySwingSound(this, index, (void*)(soundStrings + kTextSwingMissWood));
        } else {
            if (InstanceTable_GetClubCategory(InstanceTable_Get()) == 1)
                CGolfPlayer::PlaySwingSound(this, index, (void*)(soundStrings + kTextSwingMissIron));
        }
        if (InstanceTable_GetClubCategory(InstanceTable_Get()) != 0) {
            if (InstanceTable_GetClubCategory(InstanceTable_Get()) != 1)
                return;
        }
        if (*(int*)((char*)GolfDoc_GetShotGaugeData() + 0x10) >= 2)
            return;
        IActor_DispatchMessage(this, this, golfPlayerStrings + kTextGallery, 7, 0, 0, 0, 0);
        return;
    }
    unsigned char hole = ((CGolfDoc*)GolfDoc_GetCurrent())->GetCurrentHoleIndex();
    int holeStrokeCount =
        *(unsigned char*)((char*)GolfPlayer_GetCurrentEntryByIndex((unsigned char)index) + hole + 0x28);
    if ((int)InstanceTable_GetPowerShotModeWord(InstanceTable_Get()))
        CGolfPlayer::PlaySwingSound(this, index, (void*)(soundStrings + kTextSwingPowershotEffect));
    if (AsCGolfDoc(GolfDoc_GetCurrent())->alternateMotionPowerDecided) {
        int ruleSoundState = 0;
        IActor_DispatchMessage(this, this, golfPlayerStrings + kTextGolfRule, 0x26,
                               (unsigned int)((int)&ruleSoundState), 0, 0, 0);
        switch (ruleSoundState) {
        case 0:
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPretenseWoodSmall));
            return;
        case 1:
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPretenseWoodMiddle));
            return;
        case 2:
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPretenseWoodLarge));
            return;
        default:
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPretenseWoodSmall));
            return;
        }
    }
    float shotPower = kFloat_0_GolfPlayer;
    IActor_DispatchMessage(this, this, golfPlayerStrings + kTextGolfRule, 0x10, (unsigned int)((int)&shotPower), 0, 0,
                           0);
    if (shotPower > kStrongSwingSoundPower_GolfPlayer)
        soundStrength = 1;
    else
        soundStrength = 2;
    int ruleSoundState = 0;
    IActor_DispatchMessage(this, this, golfPlayerStrings + kTextGolfRule, 0x27, (unsigned int)((int)&ruleSoundState), 0,
                           0, 0);
    usesNoveltyClub = 0;
    char* entry = (char*)SharedDoc_GetCurrent() + AsCGolfDoc(GolfDoc_GetCurrent())->currentPlayer * 0x1D4;
    if (ItemInventory_FindClubSetIndex(SharedDoc_GetItemInventory(), *(int*)(entry + 0x1CC)) == 6)
        usesNoveltyClub = 1;
    unsigned char clubCategory = InstanceTable_GetClubCategory(InstanceTable_Get());
    switch (clubCategory) {
    case 0:
        if (!usesNoveltyClub) {
            if (holeStrokeCount == 1) {
                if (soundStrength == 1)
                    CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingTeeS));
                else
                    CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingTeeW));
            } else {
                if (soundStrength == 1)
                    CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingDriveS));
                else
                    CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingDriveW));
            }
        } else if (usesNoveltyClub == 1) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextPicohanWood));
        }
        if (*(int*)((char*)GolfBall__Fv() + 0x44) & 0x10) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingTomahawk));
            if (!ruleSoundState) {
                Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextFlyTomahawk, 0, 0, 0, 0,
                                 kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
                return;
            }
        } else if (*(int*)((char*)GolfBall__Fv() + 0x44) & 0x20) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingCobra));
            if (!ruleSoundState) {
                Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextFlyCobra, 0, 0, 0, 0, kFloat_0_5_GolfPlayer,
                                 kFloat_200_GolfSound, kFloat_1_GolfPlayer);
                return;
            }
        } else if (*(int*)((char*)GolfBall__Fv() + 0x44) & 0x40) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingCobra));
            Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextFlyCobra, 0, 0, 0, 0, kFloat_0_5_GolfPlayer,
                             kFloat_200_GolfSound, kFloat_1_GolfPlayer);
        }
        return;
    case 1:
        if (*(int*)((char*)GolfBall__Fv() + 0x44) & 0x10) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingTomahawk));
            if (!ruleSoundState)
                Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextFlyTomahawk, 0, 0, 0, 0,
                                 kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
        }
        if (!usesNoveltyClub) {
            if (soundStrength == 1) {
                CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingNormalS));
            } else {
                Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextSwingNormalW, 0, 0, 0, 0,
                                 kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
                CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingNormalW));
            }
        } else if (usesNoveltyClub == 1) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextPicohanIron));
        }
        switch (*(unsigned char*)((char*)GolfBall__Fv() + 0x59)) {
        case 1:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronFairwaySoundFormat, soundStrength)));
            break;
        case 3:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronBunkerSoundFormat, soundStrength)));
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextBunkerSandEffect));
            break;
        case 4:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronLeavesSoundFormat, soundStrength)));
            break;
        }
        CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPutting));
        return;
    case 2:
        if (*(int*)((char*)GolfBall__Fv() + 0x44) & 0x10) {
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingTomahawk));
            if (!ruleSoundState)
                Sound_PlayByName(g_soundManager, golfPlayerStrings + kTextFlyTomahawk, 0, 0, 0, 0,
                                 kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer);
        }
        if (usesNoveltyClub == 1)
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextMogura));
        switch (*(unsigned char*)((char*)GolfBall__Fv() + 0x59)) {
        case 1:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronFairwaySoundFormat, soundStrength)));
            break;
        case 3:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronBunkerSoundFormat, soundStrength)));
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextBunkerSandEffect));
            break;
        case 4:
            CGolfPlayer::PlaySwingSound(
                this, index,
                (void*)(String_FormatTemporary(golfPlayerStrings + kTextIronLeavesSoundFormat, soundStrength)));
            break;
        }
        /* fall through */
    case 3:
        if (!usesNoveltyClub)
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextSwingPutting));
        else if (usesNoveltyClub == 1)
            CGolfPlayer::PlaySwingSound(this, index, (void*)(golfPlayerStrings + kTextHarisenPutt));
        break;
    default:
        return;
    }
}

#pragma dont_inline off

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Sample world-space terrain at origin + headingPath*fraction. Scratch and output
// vectors belong to the caller; separate coarse/fine slots retain the matched frame.
static inline void sampleSwingContactTerrain(V3& scaledPath, V3& worldPoint, V3& queryCopy, V3& terrainHit,
                                             V3& acceptedPoint, V3& headingPath, char* contactFrameBytes,
                                             float fraction) {
    __ml__FfRC7WVector(&scaledPath, &headingPath, fraction);
    __pl__FRC7WVectorRC7WVector(&worldPoint, &((SwingContactFrame*)contactFrameBytes)->position, &scaledPath);
    *(I3*)&queryCopy = *(I3*)&worldPoint;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&terrainHit, CQuadTree_GetInstance(), &queryCopy, 1, 0, 0);
    __as__7WVectorFRC7WVector(&acceptedPoint, &terrainHit);
}

// Find the terrain contact basis, publish swing clearance, and cache the ground slope.
void CGolfPlayer::UpdateSwingContactData(void* playerObject, int requestedPlayerIndex) {
    // Negative index selects the current slot. Positions/path/hits are world
    // space; sampleFraction is dimensionless along a heading basis * swingReach.
    // Contact matrix +0x24 translation and the pose position at matrix+0x30
    // are separate. The latter is explicitly added after transforming grip data.
    char* contactFrameBytes;
    char* poseBytes;
    char* slopeRecordBytes;
    int contactFound;
    int playerIndex;
    float sampleFraction;
    float coarseIntervalEnd;
    float zero;
    struct SearchTemps {
        float distance[2];
        V3 radius0, radius1;
        V3 transformed, projected, projectedCopy, cross, direction;
        V3 fineDelta1, fineDelta0;
        V3 fineScale1, fineSum1, fineCopy1, fineHit1;
        V3 fineScale0, fineSum0, fineCopy0, fineHit0;
        V3 coarseDelta1, coarseDelta0;
        V3 coarseScale1, coarseSum1, coarseCopy1, coarseHit1;
        V3 coarseScale0, coarseSum0, coarseCopy0, coarseHit0;
        V3 radius2;
        V3 point1, point0, path;
        float unused;
        Mat fallback, origin;
    } s;
    char* self = (char*)playerObject;
    playerIndex = requestedPlayerIndex;
    if (playerIndex < 0)
        playerIndex = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
    int resolved = playerIndex;
    if (resolved < 0)
        resolved = currentPlayerIndex();
    char* items = ((char*)AsCGolfPlayer(self)->playerStates);
    contactFrameBytes = items + resolved * kPlayerPoseStride + 0x10;
    int resolved2 = playerIndex;
    if (resolved2 < 0)
        resolved2 = currentPlayerIndex();
    poseBytes = items + resolved2 * kPlayerPoseStride;
    int resolved3 = playerIndex;
    if (resolved3 < 0)
        resolved3 = currentPlayerIndex();
    slopeRecordBytes = items + resolved3 * kPlayerPoseStride;
    Matrix_SetAxisRotation(&s.origin, ((PlayerPoseState*)poseBytes)->headingRadians, 1);
    __ml__FfRC7WVector(&s.path, &s.origin, AsCGolfPlayer(self)->swingReach);
    sampleFraction = kFloat_0_GolfPlayer;
    contactFound = 0;
    zero = sampleFraction;
    coarseIntervalEnd = kCoarseSwingContactStep_GolfPlayer;
    s.radius2.x = AsCGolfPlayer(self)->swingReach;
    float searchEnd = kFloat_1_GolfPlayer;
    s.radius0.y = s.radius2.x;
    s.radius1.z = s.radius2.x;
    // Search contract: coarse pairs are [fraction, fraction+coarseStep] while
    // fraction<1. Fine search then covers that last coarse interval, even when
    // coarse search ran off the end. Accept the FIRST pair whose signed radial
    // errors have product<=0, using its SECOND terrain point to build the basis.
    // If refinement accepts no pair, restore the heading basis and block placement.
    // Coarse search: bracket the terrain crossing of the swing-radius sphere.
    while (sampleFraction < searchEnd) {
        sampleSwingContactTerrain(s.coarseScale0, s.coarseSum0, s.coarseCopy0, s.coarseHit0, s.point0, s.path,
                                  contactFrameBytes, sampleFraction);
        sampleSwingContactTerrain(s.coarseScale1, s.coarseSum1, s.coarseCopy1, s.coarseHit1, s.point1, s.path,
                                  contactFrameBytes, coarseIntervalEnd + sampleFraction);
        __mi__FRC7WVectorRC7WVector(&s.coarseDelta0, &s.point0, &((SwingContactFrame*)contactFrameBytes)->position);
        s.distance[0] = WVectorLen__FRC7WVector(&s.coarseDelta0) - AsCGolfPlayer(self)->swingReach;
        __mi__FRC7WVectorRC7WVector(&s.coarseDelta1, &s.point1, &((SwingContactFrame*)contactFrameBytes)->position);
        s.distance[1] = WVectorLen__FRC7WVector(&s.coarseDelta1) - AsCGolfPlayer(self)->swingReach;
        if (s.distance[0] * s.distance[1] <= zero)
            break;
        sampleFraction += coarseIntervalEnd;
    }
    float fineStep = kFineSwingContactStep_GolfPlayer;
    coarseIntervalEnd = kCoarseSwingContactStep_GolfPlayer + sampleFraction;
    zero = kFloat_0_GolfPlayer;
    // Refine the bracket, then build the contact basis and test the grip position.
    while (sampleFraction < coarseIntervalEnd) {
        sampleSwingContactTerrain(s.fineScale0, s.fineSum0, s.fineCopy0, s.fineHit0, s.point0, s.path,
                                  contactFrameBytes, sampleFraction);
        sampleSwingContactTerrain(s.fineScale1, s.fineSum1, s.fineCopy1, s.fineHit1, s.point1, s.path,
                                  contactFrameBytes, fineStep + sampleFraction);
        __mi__FRC7WVectorRC7WVector(&s.fineDelta0, &s.point0, &((SwingContactFrame*)contactFrameBytes)->position);
        s.distance[0] = WVectorLen__FRC7WVector(&s.fineDelta0) - AsCGolfPlayer(self)->swingReach;
        __mi__FRC7WVectorRC7WVector(&s.fineDelta1, &s.point1, &((SwingContactFrame*)contactFrameBytes)->position);
        s.distance[1] = WVectorLen__FRC7WVector(&s.fineDelta1) - AsCGolfPlayer(self)->swingReach;
        if (s.distance[0] * s.distance[1] <= zero) {
            __mi__FRC7WVectorRC7WVector(&s.direction, &s.point1, &((SwingContactFrame*)contactFrameBytes)->position);
            __as__7WVectorFRC7WVector(contactFrameBytes, &s.direction);
            Normalize__7WVectorFv(contactFrameBytes);
            Vector_CrossProduct(&s.cross, &((SwingContactFrame*)contactFrameBytes)->axisZ, contactFrameBytes);
            __as__7WVectorFRC7WVector(&((SwingContactFrame*)contactFrameBytes)->axisY, &s.cross);
            contactFound = 1;
            void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(manager)->unk_3CC)
                ((SwingObstructionView*)self)->swingPlacementBlocked = 0;
            else {
                float angle = ((SwingContactFrame*)contactFrameBytes)->axisX.y;
                angle = angle > kFloat_0_GolfPlayer ? angle : -angle;
                if (angle > Math_SinApprox(kMaximumGroundSlopeRadians_GolfPlayer))
                    ((SwingObstructionView*)self)->swingPlacementBlocked = 1;
                else {
                    void* view = InstanceTable_Get();
                    char* base = self + playerIndex * 0x40;
                    Matrix_TransformPoint(&s.transformed,
                                          base + *(unsigned char*)((char*)*(void**)((char*)view + 4) + 0xA) * 0x10 +
                                              0x128,
                                          contactFrameBytes);
                    __pl__FRC7WVectorRC7WVector(&s.projected, &((SwingContactFrame*)contactFrameBytes)->position,
                                                &s.transformed);
                    *(I3*)&s.projectedCopy = *(I3*)&s.projected;
                    ((SwingObstructionView*)self)->swingPlacementBlocked =
                        (unsigned char)((int (*)(void*, const void*, float))CQuadTree_OverlappedWithObject)(
                            CQuadTree_GetInstance(), &s.projectedCopy, kSwingGripClearance_GolfPlayer);
                }
            }
            IActor_DispatchMessage(self, self, golfPlayerStrings + kTextGolfRule, 6,
                                   (unsigned int)(((SwingObstructionView*)self)->swingPlacementBlocked == 0), 0, 0, 0);
            ((PlayerPoseState*)slopeRecordBytes)->groundSlopeRadians =
                kGroundSlopeSign_GolfPlayer * (float)asin(((SwingContactFrame*)contactFrameBytes)->axisX.y);
            break;
        }
        sampleFraction += fineStep;
    }
    // A missing crossing falls back to the heading basis and blocks the swing placement.
    if (!contactFound) {
        Matrix_SetAxisRotation(&s.fallback, ((PlayerPoseState*)poseBytes)->headingRadians, 1);
        Matrix_CopyAffine(contactFrameBytes, &s.fallback);
        ((SwingObstructionView*)self)->swingPlacementBlocked = 1;
        IActor_DispatchMessage(self, self, golfPlayerStrings + kTextGolfRule, 6,
                               (unsigned int)(((SwingObstructionView*)self)->swingPlacementBlocked == 0), 0, 0, 0);
        ((PlayerPoseState*)slopeRecordBytes)->groundSlopeRadians = kFloat_0_GolfPlayer;
    }
    char* animationBytes = ((char*)AsCGolfPlayer(self)->animationStates) + playerIndex * kAnimationStateStride;
    *(int*)&((PlayerAnimationRecord*)animationBytes)->flags |=
        (kAnimationTransformDirty | kAnimationPlaying | kAnimationAttachmentDirty);
}

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
// Return one for a candidate with acceptable contact slope and unobstructed grip placement.
int CGolfPlayer::IsSwingPlacementUsable(void* playerObject, const void* originPosition, float headingRadians) {
    const float* swingOrigin = (const float*)originPosition;
    Mat contactBasis;
    float radialError[2];
    // originPosition is a borrowed world-space swing origin; heading is radians.
    // Return 1 means clear/usable, 0 means missing terrain, no crossing or blocked
    // placement. The query also updates quadtree nearby-object state; not pure.
    char* self = (char*)playerObject;

    Mat headingBasis;
    Bounds bounds;
    Bounds boundsCopy;
    V3 reachVector;
    V3 terrainAtStart;
    V3 terrainAtEnd;
    V3 unusedRadiusX;
    V3 scaledHeadingAxis;
    V3 coarseGroundStart, coarseQueryStartBits, coarseQueryStart, coarseOffsetStart, coarseGroundEnd,
        coarseQueryEndBits, coarseQueryEnd, coarseOffsetEnd, coarseOriginToStart, coarseOriginToEnd;
    V3 fineGroundStart, fineQueryStartBits, fineQueryStart, fineOffsetStart, fineGroundEnd, fineQueryEndBits,
        fineQueryEnd, fineOffsetEnd, fineOriginToStart, fineOriginToEnd;
    V3 originToContact, contactYAxis, queryMinimum, minimumMargin, horizontalMinimumMask, queryMaximum, maximumMargin,
        horizontalMaximumMask;
    V3 gripWorldPositionBits, gripContactOffset, gripWorldPosition;
    V3 unusedRadiusZ, unusedRadiusY;

    Matrix_SetAxisRotation(&headingBasis, headingRadians, 1);
    __ml__FfRC7WVector(&scaledHeadingAxis, &headingBasis, AsCGolfPlayer(self)->swingReach);
    __as__7WVectorFRC7WVector(&reachVector, &scaledHeadingAxis);

    float sampleFraction = kFloat_0_GolfPlayer;
    float coarseStepThenEnd = kCoarseSwingContactStep_GolfPlayer;
    float coarseZero = sampleFraction;
    float searchEnd = kFloat_1_GolfPlayer;
    float swingRadius = AsCGolfPlayer(self)->swingReach;

    unusedRadiusX.x = swingRadius;

    unusedRadiusY.y = swingRadius;
    unusedRadiusZ.z = swingRadius;
    // Search t along origin + t*reachVector, with dimensionless t in [0,1).
    // radialError = distance(ground(origin+t*reachVector), origin) - swingReach.
    // A sign change brackets contact with the sphere centered at the origin.
    // Copy through I3 preserves XYZ float bits for the by-value ground-query ABI;
    // it does not convert coordinates to integers. Distances retain game world units.
    // coarseStepThenEnd starts as a step and becomes the bracket end after this loop.
    while (sampleFraction < searchEnd) {
        __ml__FfRC7WVector(&coarseOffsetStart, &reachVector, sampleFraction);
        __pl__FRC7WVectorRC7WVector(&coarseQueryStart, swingOrigin, &coarseOffsetStart);
        *(I3*)&coarseQueryStartBits = *(I3*)&coarseQueryStart;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&coarseGroundStart, CQuadTree_GetInstance(), &coarseQueryStartBits, 1,
                                                0, 0);
        __as__7WVectorFRC7WVector(&terrainAtStart, &coarseGroundStart);
        __ml__FfRC7WVector(&coarseOffsetEnd, &reachVector, (coarseStepThenEnd + sampleFraction));
        __pl__FRC7WVectorRC7WVector(&coarseQueryEnd, swingOrigin, &coarseOffsetEnd);
        *(I3*)&coarseQueryEndBits = *(I3*)&coarseQueryEnd;
        int groundObjectIndex;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&coarseGroundEnd, CQuadTree_GetInstance(), &coarseQueryEndBits, 1,
                                                (int)&groundObjectIndex, 0);
        __as__7WVectorFRC7WVector(&terrainAtEnd, &coarseGroundEnd);
        if (groundObjectIndex < 0)
            return 0;
        __mi__FRC7WVectorRC7WVector(&coarseOriginToStart, &terrainAtStart, swingOrigin);
        radialError[0] = WVectorLen__FRC7WVector(&coarseOriginToStart) - AsCGolfPlayer(self)->swingReach;
        __mi__FRC7WVectorRC7WVector(&coarseOriginToEnd, &terrainAtEnd, swingOrigin);
        char* savedSelf = self;
        radialError[1] = WVectorLen__FRC7WVector(&coarseOriginToEnd) - AsCGolfPlayer(savedSelf)->swingReach;
        if (radialError[0] * radialError[1] <= coarseZero)
            break;
        float nextSampleFraction = sampleFraction + coarseStepThenEnd;
        sampleFraction = nextSampleFraction;
    }

    char* activePoseBytes =
        ((char*)AsCGolfPlayer(self)->playerStates) +
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer * kPlayerPoseStride;
    coarseStepThenEnd = kCoarseSwingContactStep_GolfPlayer + sampleFraction;
    float fineStep = kFineSwingContactStep_GolfPlayer;
    float fineZero = kFloat_0_GolfPlayer;

    // Fine samples validate the contact slope and the transformed grip clearance.
    while (sampleFraction < coarseStepThenEnd) {
        __ml__FfRC7WVector(&fineOffsetStart, &reachVector, sampleFraction);
        __pl__FRC7WVectorRC7WVector(&fineQueryStart, swingOrigin, &fineOffsetStart);
        *(I3*)&fineQueryStartBits = *(I3*)&fineQueryStart;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&fineGroundStart, CQuadTree_GetInstance(), &fineQueryStartBits, 1, 0,
                                                0);
        __as__7WVectorFRC7WVector(&terrainAtStart, &fineGroundStart);
        __ml__FfRC7WVector(&fineOffsetEnd, &reachVector, fineStep + sampleFraction);
        __pl__FRC7WVectorRC7WVector(&fineQueryEnd, swingOrigin, &fineOffsetEnd);
        *(I3*)&fineQueryEndBits = *(I3*)&fineQueryEnd;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&fineGroundEnd, CQuadTree_GetInstance(), &fineQueryEndBits, 1, 0, 0);
        __as__7WVectorFRC7WVector(&terrainAtEnd, &fineGroundEnd);
        __mi__FRC7WVectorRC7WVector(&fineOriginToStart, &terrainAtStart, swingOrigin);
        radialError[0] = WVectorLen__FRC7WVector(&fineOriginToStart) - AsCGolfPlayer(self)->swingReach;
        __mi__FRC7WVectorRC7WVector(&fineOriginToEnd, &terrainAtEnd, swingOrigin);
        radialError[1] = WVectorLen__FRC7WVector(&fineOriginToEnd) - AsCGolfPlayer(self)->swingReach;
        if (radialError[0] * radialError[1] <= fineZero) {
            Matrix_SetAxisRotation(&contactBasis, ((PlayerPoseState*)activePoseBytes)->headingRadians, 1);
            __mi__FRC7WVectorRC7WVector(&originToContact, &terrainAtEnd, swingOrigin);
            __as__7WVectorFRC7WVector(&contactBasis, &originToContact);
            Normalize__7WVectorFv(&contactBasis);
            Vector_CrossProduct(&contactYAxis, &contactBasis.v[6], &contactBasis.v[0]);
            __as__7WVectorFRC7WVector(&contactBasis.v[3], &contactYAxis);
            Vec3_Zero(&bounds.min);
            Vec3_Zero(&bounds.max);
            horizontalMinimumMask.x = kFloat_1_GolfPlayer;
            horizontalMinimumMask.y = kFloat_0_GolfPlayer;
            horizontalMinimumMask.z = kFloat_1_GolfPlayer;
            __ml__FfRC7WVector(&minimumMargin, &horizontalMinimumMask, kFloat_16_0);
            void* ballForHistory = GolfBall__Fv();
            __mi__FRC7WVectorRC7WVector(
                &queryMinimum,
                PreData__9CGolfBallFi(GolfBall__Fv(), ((BallHistoryView*)ballForHistory)->activeHistoryIndex),
                &minimumMargin);
            __as__7WVectorFRC7WVector(&bounds.min, &queryMinimum);
            horizontalMaximumMask.x = kFloat_1_GolfPlayer;
            horizontalMaximumMask.y = kFloat_0_GolfPlayer;
            horizontalMaximumMask.z = kFloat_1_GolfPlayer;
            __ml__FfRC7WVector(&maximumMargin, &horizontalMaximumMask, kFloat_16_0);
            ballForHistory = GolfBall__Fv();
            __pl__FRC7WVectorRC7WVector(
                &queryMaximum,
                PreData__9CGolfBallFi(GolfBall__Fv(), ((BallHistoryView*)ballForHistory)->activeHistoryIndex),
                &maximumMargin);
            __as__7WVectorFRC7WVector(&bounds.max, &queryMaximum);
            boundsCopy = bounds;
            ((CQuadTree*)CQuadTree_GetInstance())->GetObjList((float*)&boundsCopy);

            int placementBlocked = 1;
            float verticalComponent = ((((contactBasis.v[1]))));
            verticalComponent = verticalComponent > kFloat_0_GolfPlayer ? verticalComponent : -verticalComponent;
            if (!(verticalComponent > Math_SinApprox(kMaximumGroundSlopeRadians_GolfPlayer))) {
                void* shotInstances = InstanceTable_Get();

                unsigned char clubCategory;
                clubCategory = *(unsigned char*)((char*)*(void**)((char*)shotInstances + 4) + 0xA);
                Matrix_TransformPoint(
                    &gripContactOffset,
                    self + AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer * 0x40 +
                        clubCategory * 0x10 + 0x128,
                    &contactBasis);
                __pl__FRC7WVectorRC7WVector(&gripWorldPosition, swingOrigin, &gripContactOffset);
                *(I3*)&gripWorldPositionBits = *(I3*)&gripWorldPosition;
                if (((int (*)(void*, const void*, float))CQuadTree_OverlappedWithObject)(
                        CQuadTree_GetInstance(), &gripWorldPositionBits, kSwingGripClearance_GolfPlayer) == 0)
                    placementBlocked = 0;
            } else {
            }
            // Despite the legacy function name, a clear placement returns one.
            return 0 == placementBlocked;
        }
        sampleFraction = sampleFraction + fineStep;
    }
    return 0;
}

// Install a preloaded avatar, rebuild its swing caches, and release the asynchronous loader.
void CGolfPlayer::ApplyPendingAvatarLoad() {
    // Phase 1: guard the selected slot, load/retry its model and bind its scene entry.
    // Caller coordinates completion polling; no wait or loader-done test is added.
    int index;
    int modelOffsetOrDefaultClubId;
    int characterSelectionAddress;
    int avatarLoadKind;
    int playerPuppetAddress;
    int sharedPlayerOffset;
    void* sceneManager;
    void* shadowManager;
    void* activeShadowManager;
    void* clubPlacement;
    int n;
    int animationPuppetAddress;
    int characterDataAddress;
    int defaultClubModelsAddress;
    int clubModelsAddress;
    if (preloadPlayerIndex == -1)
        return;
    if (ResourceModel801B_HasModelAndParts((char*)playerModels + preloadPlayerIndex * kPlayerModelStride))
        return;

    characterSelectionAddress =
        (int)((CGolfDoc*)(g_sharedDoc->golfDoc))->GetItemSelection((unsigned char)preloadPlayerIndex);
    index = preloadPlayerIndex;
    avatarLoadKind = (*(int*)&avatarPreloadKind);
    void* animation = (void*)GolfPlayer_GetEntryByIndex((CGolfDoc*)g_sharedDoc->golfDoc, (unsigned char)index);
    if (*(int*)((char*)animation + 8) != 3) {
        modelOffsetOrDefaultClubId = index * kPlayerModelStride;
        sharedPlayerOffset = index * 0x1D4;
        if (!AvatarModel_LoadCharacterResources(
                (char*)playerModels + modelOffsetOrDefaultClubId, (void*)(characterSelectionAddress),
                (char*)g_sharedDoc + sharedPlayerOffset + 0x190, 1, avatarLoadKind, 1, index, 0)) {
            ItemSelection_RefreshPrimaryItems((void*)characterSelectionAddress);
            AvatarModel_LoadCharacterResources(
                (char*)playerModels + modelOffsetOrDefaultClubId, (void*)(characterSelectionAddress),
                (char*)g_sharedDoc + sharedPlayerOffset + 0x190, 1, avatarLoadKind, 0, index, 0);
        }
        UpdatePuppetBoundsForAnimationList((char*)playerModels + modelOffsetOrDefaultClubId);
        ((GolfPlayerAnimationState*)((char*)animationStates + index * kAnimationStateStride))->animationMode =
            kAnimationReady;
        ((GolfPlayerAnimationState*)((char*)animationStates + index * kAnimationStateStride))->pendingAnimationName = 0;
        sceneManager = g_sceneManager;
        playerPuppetAddress = (int)PlayerModel_GetPuppet((char*)playerModels + modelOffsetOrDefaultClubId);
        int playerSceneNameAddress = (int)String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, index);
        DrawScene_SetNamedObjectPuppet(sceneManager, (const char*)(playerSceneNameAddress),
                                       (void*)(playerPuppetAddress));
        if (!Sing_805DC7B0()) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                         s_renderPacketDtorRegistration_golfPlayer);
                s_renderPacketInitialized = 1;
            }
            Sing_805DC7B0() = s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, (const char*)((int)(golfPlayerStrings + kTextChar)), kFloat_0_05_PlayerShadow, 0x10,
            (const char*)((int)(golfPlayerStrings + kTextSun)), (char*)Sing_805DC7B0() + 0x4C);
        shadowManager = g_shadowManager;
        ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
            shadowManager, (const char*)((int)(golfPlayerStrings + kTextChar)),
            (void*)((int)PlayerModel_GetPuppet((char*)playerModels + modelOffsetOrDefaultClubId)));
    }

    // Phase 2: rebuild selected/default club placements and elapsed *shot keys.
    animationPuppetAddress = (int)ObjectList_GetSelected((char*)playerModels + preloadPlayerIndex * kPlayerModelStride);
    int clubInstanceTableAddress = (int)InstanceTable_Get();
    int clubSetItemId = *(int*)((char*)g_sharedDoc + preloadPlayerIndex * 0x1D4 + 0x1CC);
    clubModelsAddress = (int)ClubModel_FindCachedForItem((void*)(clubInstanceTableAddress), (void*)(clubSetItemId));
    modelOffsetOrDefaultClubId = (int)ItemInventory_GetClubSetIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, 0);
    int defaultInstanceTableAddress = (int)InstanceTable_Get();
    defaultClubModelsAddress =
        (int)ClubModel_FindCachedForItem((void*)(defaultInstanceTableAddress), (void*)(modelOffsetOrDefaultClubId));
    characterDataAddress = (int)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                                (void*)(*(int*)characterSelectionAddress));
    clubPlacement = (char*)((PlayerSwingPlacementView*)this)->byPlayerAndClub + (preloadPlayerIndex << 6);
    for (n = 0; n < 4; ++n, clubPlacement = (char*)clubPlacement + sizeof(SwingPlacement))
        ReadClubSwingPlacement((void*)(animationPuppetAddress), (void*)(clubModelsAddress),
                               (void*)(characterDataAddress), n, clubPlacement);
    ReadClubSwingPlacement((void*)(animationPuppetAddress), (void*)(defaultClubModelsAddress),
                           (void*)(characterDataAddress), 0,
                           (char*)((PlayerSwingPlacementView*)this)->defaultByPlayer + preloadPlayerIndex * 0x10);
    CacheShotKeyTimes(preloadPlayerIndex);
    int current = AsCGolfDoc((CGolfDoc*)g_sharedDoc->golfDoc)->currentPlayer;
    activeShadowManager = g_shadowManager;
    ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
        activeShadowManager, (const char*)((int)(golfPlayerStrings + kTextChar)),
        (void*)((int)PlayerModel_GetPuppet((char*)playerModels + current * kPlayerModelStride)));
    // Phase 3: release the asynchronous loader and clear the selected-load slot.
    U80146020_Obj* old = (U80146020_Obj*)avatarPreloader;
    if (old) {
        delete old;
        avatarPreloader = 0;
    }
    preloadPlayerIndex = -1;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Load and expose an extra player slot for a scripted scene.
void CGolfPlayer::AddPlayerSlot(unsigned short id) {
    // Caller guarantees regular+extra < 4. id is forwarded through the legacy
    // enemy-slot API (whose name-based declaration disagrees across units).
    // Phase 1: reserve the next scripted slot and resolve outfit/rank selection.
    void* sel;
    int characterSelectionAddress;
    int playerIndex;
    void* playerModel;
    SharedOutfitSelection* panel;
    unsigned char extraPlayerCount;
    unsigned char regularPlayerCount;
    int rankTier;
    unsigned char packedOutfitSelection;
    unsigned int sceneNameAddress;
    void* record;
    int characterId;

    extraPlayerCount = playerIndexOffset;
    regularPlayerCount = ((U80146020_Entry*)(CGolfDoc*)g_sharedDoc->golfDoc)->count;
    playerIndexOffset = extraPlayerCount + 1;
    playerIndex = extraPlayerCount + regularPlayerCount;
    SharedDoc_LoadEnemyPlayerSlotByName(playerIndex, id); /* 0x801463B4: r4 = the incoming id, not set here */
    characterSelectionAddress = (int)((CGolfDoc*)(g_sharedDoc->golfDoc))->GetItemSelection((unsigned char)playerIndex);
    sel = (char*)g_sharedDoc + playerIndex * 0x1D4;
    playerModel = (char*)playerModels + playerIndex * kPlayerModelStride;
    panel = &((SharedPlayerSelectionView*)sel)->outfit;
    rankTier = (int)PlayerRank_FindTierForValue(((SharedPlayerSelectionView*)sel)->rankValue);
    packedOutfitSelection = panel->packedOutfitSelection;
    ItemSelection_BuildCharacterOutfit(
        (void*)(characterSelectionAddress), panel->characterId, (unsigned int)((packedOutfitSelection >> 3) & 31),
        (unsigned int)(packedOutfitSelection & 7), panel->partSelections, (unsigned char)(rankTier & 255));
    // Phase 2: load the model, retrying refreshed primary items on failure.
    if (!AvatarModel_LoadCharacterResources(playerModel, (void*)(characterSelectionAddress),
                                            ((SharedPlayerSelectionView*)sel)->avatarParameters, 1, 3, 1, -1, 0)) {
        ItemSelection_RefreshPrimaryItems((void*)characterSelectionAddress);
        AvatarModel_LoadCharacterResources(playerModel, (void*)(characterSelectionAddress),
                                           ((SharedPlayerSelectionView*)sel)->avatarParameters, 1, 3, 0, -1, 0);
    }
    UpdatePuppetBoundsForAnimationList(playerModel);
    sel = (void*)golfPlayerStrings;
    CGolfPlayer_SetPlayerAnimation(this, (char*)sel + kTextMTSTORYSTAND, kFloat_0_GolfPlayer, playerIndex);
    *(int*)((char*)animationStates + playerIndex * kAnimationStateStride + 0x60) = 0;
    *((unsigned char*)animationStates + playerIndex * kAnimationStateStride + 0x39) = 0;
    sceneNameAddress = (int)String_FormatTemporary((char*)sel + kTextPLAYERD, playerIndex);
    sel = (void*)g_sceneManager;
    DrawScene_SetNamedObjectPuppet(sel, (const char*)(sceneNameAddress),
                                   (void*)((int)PlayerModel_GetPuppet(playerModel)));
    CacheSwingPlacements(-1);
    if (!((int&)GetConteModelSlotsSlot())) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty((const char*)(s_conteModelSlots));
            s_courseSlotTableInitialized = 1;
        }
        ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
    }
    // Phase 3: publish model/animation pointers into the scripted slot table.
    record = SlotTable_GetEntryByte((void*)((int&)GetConteModelSlotsSlot()), playerIndex);
    characterId = *(int*)panel;
    *(unsigned char*)record = (unsigned char)characterId;
    *(int*)((char*)record + 4) = (int)PlayerModel_GetPuppet(playerModel);
    *(int*)((char*)record + 8) = (int)ObjectList_GetSelected(playerModel);
    SetSlotEnabled(playerIndex, 1, 1);
}
#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Switch normal players into/out of scripted slots and update their scene/shadow bindings.
void CGolfPlayer::SetCutsceneSlotsEnabled(int cutsceneSlotsEnabled) {
    // Phase 1: remove normal shadow groups on entry; recreate them on exit.
    void* sceneManager;
    int playerModelOffset;
    int playerIndex;
    unsigned int sceneNameAddress;
    int slotIndexOrSharedPlayerOffset;
    float sceneDistance;

    if (cutsceneSlotsEnabled) {
        GfxObjList_RemoveGroupByName((unsigned int)(g_shadowManager),
                                     (const char*)((int)(golfPlayerStrings + kTextChar)));
        GfxObjList_RemoveGroupByName((unsigned int)(g_shadowManager),
                                     (const char*)((int)(golfPlayerStrings + kTextCaddie)));
        GfxObjList_RemoveGroupByName((unsigned int)(g_shadowManager),
                                     (const char*)((int)(golfPlayerStrings + kTextClub)));
    } else {
        if (Sing_805DC7B0() == 0) {
            if (s_renderPacketInitialized == 0) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                         s_renderPacketDtorRegistration_golfPlayer);
                s_renderPacketInitialized = 1;
            }
            Sing_805DC7B0() = s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, (const char*)((int)(golfPlayerStrings + kTextChar)), kFloat_0_05_PlayerShadow, 0x10,
            (const char*)((int)(golfPlayerStrings + kTextSun)), (char*)Sing_805DC7B0() + 0x4C);
        if (Sing_805DC7B0() == 0) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                         s_renderPacketDtorRegistration_golfPlayer);
                s_renderPacketInitialized = 1;
            }
            Sing_805DC7B0() = s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, (const char*)((int)(golfPlayerStrings + kTextCaddie)), kCaddieShadowParameter_GolfPlayer,
            0x10, (const char*)((int)(golfPlayerStrings + kTextSun)), (char*)Sing_805DC7B0() + 0x4C);
        if (!Sing_805DC7B0()) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                         s_renderPacketDtorRegistration_golfPlayer);
                s_renderPacketInitialized = 1;
            }
            Sing_805DC7B0() = s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, (const char*)((int)(golfPlayerStrings + kTextClub)), kFloat_0_05_PlayerShadow, 0x100,
            (const char*)((int)(golfPlayerStrings + kTextSun)), (char*)Sing_805DC7B0() + 0x4C);
    }
    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)((int)(golfPlayerStrings + kTextClubActor)),
                                    cutsceneSlotsEnabled == 0);

    // Phase 2: pass distance 0 for scripted slots, 640 otherwise (the setter
    // stores the square), then publish normal players in the scripted table.
    if (cutsceneSlotsEnabled)
        sceneDistance = kFloat_0_GolfPlayer;
    else
        sceneDistance = kPlayerSceneDistance640;
    slotIndexOrSharedPlayerOffset = 0;
    while (slotIndexOrSharedPlayerOffset < 4) {
        sceneManager = g_sceneManager;
        sceneNameAddress = (int)String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, slotIndexOrSharedPlayerOffset);
        NamedEffectList_SetDistance(sceneManager, (const char*)(sceneNameAddress), sceneDistance);
        slotIndexOrSharedPlayerOffset += 1;
    }
    if (0 == ((int&)GetConteModelSlotsSlot())) {
        if (!s_courseSlotTableInitialized) {
            CourseSlotTable_InitEmpty((const char*)(s_conteModelSlots));
            s_courseSlotTableInitialized = 1;
        }
        ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
    }
    void* courseSlots = (void*)((int&)GetConteModelSlotsSlot());
    slotIndexOrSharedPlayerOffset = 0;
    playerIndex = 0;
    playerModelOffset = 0;
    for (; playerIndex < ((U80146020_Entry*)(CGolfDoc*)g_sharedDoc->golfDoc)->count;
         ++playerIndex, slotIndexOrSharedPlayerOffset += 0x1D4, playerModelOffset += kPlayerModelStride) {
        void* record = SlotTable_GetEntryByte(courseSlots, playerIndex);

        *(unsigned char*)record = (unsigned char)(*(int*)((char*)g_sharedDoc + slotIndexOrSharedPlayerOffset + 0x120));

        *(int*)((char*)record + 4) =
            (cutsceneSlotsEnabled ? (int)PlayerModel_GetPuppet((char*)playerModels + playerModelOffset) : 0);

        *(int*)((char*)record + 8) =
            (cutsceneSlotsEnabled ? (int)ObjectList_GetSelected((char*)playerModels + playerModelOffset) : 0);
        SetSlotEnabled(playerIndex, cutsceneSlotsEnabled, 1);
    }
    // Phase 3: entering resets the extra-slot count; leaving hides/clears extras
    // from the highest slot downward so the normal player count remains stable.
    if (cutsceneSlotsEnabled)
        playerIndexOffset = 0;
    else
        while (playerIndexOffset) {
            playerIndexOffset -= 1;
            int id = playerIndexOffset + ((U80146020_Entry*)(CGolfDoc*)g_sharedDoc->golfDoc)->count;
            SharedStringState_DefaultHook(id);
            if (((int&)GetConteModelSlotsSlot()) == 0) {
                if (s_courseSlotTableInitialized == 0) {
                    CourseSlotTable_InitEmpty((const char*)(s_conteModelSlots));
                    s_courseSlotTableInitialized = 1;
                }
                ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
            }
            void* record = SlotTable_GetEntryByte((void*)((int&)GetConteModelSlotsSlot()), id);
            *(int*)((char*)record + 4) = 0;
            *(int*)((char*)record + 8) = 0;
            SetSlotEnabled(id, 0, 1);
        }
}

#pragma opt_common_subs off
// Select an animation range, clamp its initial time, and dispatch its initial events.
void CGolfPlayer_SetPlayerAnimation(CGolfPlayer* self, const char* name, float timeOffset, int index) {
    // name is borrowed; a negative index selects current player. timeOffset is
    // seconds: >=0 is relative to range start; <0 is relative to range end.
    // Clamp within that range. Missing name falls back to facial animation;
    // missing fallback leaves readyForQueuedAnimation set and skips evaluation.
    if (index < 0)
        index = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
    AsCGolfPlayer(self)->readyForQueuedAnimation = 1;
    int animationOffset = index * kAnimationStateStride;
    int modelOffset = index * kPlayerModelStride;
    ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->animationObject =
        (void*)ObjectList_FindAnimationRangeByName((char*)AsCGolfPlayer(self)->playerModels + modelOffset, name, 1, 0);
    if (!((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->animationObject)
        ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->animationObject =
            (void*)ObjectList_FindAnimationRangeByName((char*)AsCGolfPlayer(self)->playerModels + modelOffset,
                                                       golfPlayerStrings + kTextMTFACIALANIMATION, 1, 0);
    if (((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->animationObject) {
        ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))
            ->interpolationStart = kFloat_1_GolfPlayer;
        ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))
            ->interpolationRatePerSecond = kFloat_0_GolfPlayer;
        if (timeOffset < kFloat_0_GolfPlayer) {
            char* data = (char*)AsCGolfPlayer(self)->animationStates;
            void* animationRange = *(void**)(data + animationOffset + 4);
            float rangeStart = *(float*)animationRange;
            float value = timeOffset + *(float*)((char*)animationRange + 4);
            float result = (rangeStart < value) ? value : rangeStart;
            *(float*)(data + animationOffset) = result;
        } else {
            char* data = (char*)AsCGolfPlayer(self)->animationStates;
            void* animationRange = *(void**)(data + animationOffset + 4);
            float value = timeOffset + *(float*)animationRange;
            float result =
                (value > *(float*)((char*)animationRange + 4)) ? *(float*)((char*)animationRange + 4) : value;
            *(float*)(data + animationOffset) = result;
        }
        unsigned int* flags = (unsigned int*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset + 0x5C);
        *flags |= kAnimationTransformDirty | kAnimationPlaying | kAnimationBodyLightDirty | kAnimationClubLightDirty;
        flags = (unsigned int*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset + 0x5C);
        AsCGolfPlayer(self)->attachmentPositionLocked = (*flags >> 8) & 1;
        flags = (unsigned int*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset + 0x5C);
        *flags &= ~(kAnimationClubHidden | kAnimationUseLockedAttachment);
        PlayerModel_SetDefaultPuppetLayerEnabled((char*)AsCGolfPlayer(self)->playerModels + modelOffset, 0);
        void* selectedAnimationRange =
            ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))
                ->animationObject;
        float startTime = *(float*)selectedAnimationRange;

        AnimationEventParser_ProcessFrameRange(
            (char*)&AsCGolfPlayer(self)->sub_vtbl,
            (void*)(((int)ObjectList_GetSelected((char*)AsCGolfPlayer(self)->playerModels + modelOffset))), startTime,
            startTime, selectedAnimationRange, index, 1);
        AsCGolfPlayer(self)->readyForQueuedAnimation = 0;

        if (strcmp(name, (const char*)String_FormatTemporary(golfPlayerStrings + kTextReadyFormat,
                                                             AsCGolfPlayer(self)->shotAnimationPrefix)) == 0 ||
            strcmp(name, (const char*)String_FormatTemporary(golfPlayerStrings + kTextDefaultAnimationFormat,
                                                             AsCGolfPlayer(self)->shotAnimationPrefix)) == 0) {
            ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->flags &=
                ~kAnimationClubHidden;
            ((GolfPlayerAnimationState*)((char*)AsCGolfPlayer(self)->animationStates + animationOffset))->flags &=
                ~kAnimationUseLockedAttachment;
            DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)((int)(golfPlayerStrings + kTextClubActor)),
                                            1);
            ShadowModel_SetNamedProjectionEnabled((unsigned int)(g_shadowManager), golfPlayerStrings + kTextClub, 1);
        }
    }
}
#pragma opt_common_subs reset

#pragma opt_propagation off
// Set slot visibility, cache attachment bones, and optionally update scene/shadow bindings.
void CGolfPlayer::SetSlotEnabled(int index, int enabled, int updateScene) {
    // Requires a concrete slot even though later pose lookups resolve negatives.
    // updateScene controls extra shadows/opacity; basic PLAYER visibility changes
    // in either case. Enabling with updateScene==0 rebuilds existing item effects.
    int modelOffset = index * kPlayerModelStride;
    void* playerModels = this->playerModels;
    int playerIndex = index;
    void* animationStates = this->animationStates;
    int animationOffset = playerIndex * kAnimationStateStride;
    void* playerModel = (char*)playerModels + modelOffset;
    int enabledCopy = enabled;
    int updateSceneCopy = updateScene;
    void* animationRecord = (char*)animationStates + animationOffset;
    int ok = ResourceModel801B_HasModelAndParts(playerModel);
    if (!ok)
        return;
    int isEnabled = enabled != 0;
    ((PlayerAnimationRecord*)animationRecord)->visible = (unsigned char)isEnabled;
    if (enabled)
        WPuppet_ShowBoneSelection((void*)(int)PlayerModel_GetPuppet(playerModel), 0);
    else
        WPuppet_HideBoneSelection((void*)(int)PlayerModel_GetPuppet(playerModel), 0);
    int name = (int)String_FormatTemporary(golfPlayerStrings + kTextPLAYERD, index);
    DrawScene_SetNamedObjectEnabled(g_sceneManager, (const char*)(name), isEnabled);
    if (enabled) {
        unsigned int* state = &((PlayerAnimationRecord*)animationRecord)->flags;
        *state = (*state | 0x40) & ~0x180;
        *(unsigned char*)((PlayerAnimationRecord*)animationRecord)->pendingAnimationName = 0;
        int n = index;
        if (n < 0)
            n = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
        void* row = (char*)playerStates + n * kPlayerPoseStride;
        *(int*)&((PlayerPoseState*)row)->footBones[0] =
            (int)WPuppet_FindBoneByName((void*)(int)PlayerModel_GetPuppet(playerModel), golfPlayerStrings + kTextLFoot);
        *(int*)&((PlayerPoseState*)row)->footBones[1] =
            (int)WPuppet_FindBoneByName((void*)(int)PlayerModel_GetPuppet(playerModel), golfPlayerStrings + kTextRFoot);
        n = index;
        if (n < 0)
            n = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
        void* row2 = (char*)playerStates + n * kPlayerPoseStride;
        ((PlayerPoseState*)row2)->headRotation = kFloat_0_GolfPlayer;
        *(int*)&((PlayerPoseState*)row2)->headBone = (int)WPuppet_FindOrCacheBone(
            (void*)(int)PlayerModel_GetPuppet(playerModel), golfPlayerStrings + kTextBip01Head);
        RebuildAttachedEffects(index, (void*)(updateScene == 0), (void*)(updateScene == 0));
    }
    if (updateScene) {
        if (enabled) {
            WPuppet_HideBoneSelection((void*)(int)PlayerModel_GetPuppet(playerModel),
                                      (const char*)((int)(golfPlayerStrings + kTextBone01)));
            PuppetOwner_SetOpacityRecursive(playerModel, 0xFF);
            if (!Sing_805DC7B0()) {
                if (!s_renderPacketInitialized) {
                    RenderPacket_Init(s_renderPacket);
                    __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                             s_renderPacketDtorRegistration_golfPlayer);
                    s_renderPacketInitialized = 1;
                }
                Sing_805DC7B0() = s_renderPacket;
            }
            GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
                g_shadowManager, (const char*)(name), kFloat_0_05_PlayerShadow, 0x10,
                (const char*)((int)(golfPlayerStrings + kTextSun)), (void*)((char*)Sing_805DC7B0() + 0x4C));
            void* models = this->playerModels;
            void* shadowManager = g_shadowManager;
            ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
                shadowManager, (const char*)(name),
                (void*)((int)PlayerModel_GetPuppet((char*)models + index * kPlayerModelStride)));
            ShadowModel_SetNamedProjectionEnabled((unsigned int)(g_shadowManager), (const char*)name, 1);
        } else
            GfxObjList_RemoveGroupByName((unsigned int)(g_shadowManager), (const char*)(name));
    }
}
#pragma opt_propagation reset

#pragma use_lmw_stmw on
// Borrow the four packed affine breath matrices embedded in the player object.
struct PlayerBreathMatricesView {
    char prefix[kBreathMatrixStorageOffset];
    PlayerAffineFrameView matrices[kPlayerSlotCount];
};
typedef char PlayerBreathTranslationOffset
    [((int)DECOMP_OFFSETOF(PlayerBreathMatricesView, matrices[0].position) == kBreathMatrixTranslationOffset) ? 1 : -1];
// Rotate only breath axes; restore the saved world translation after composition.
// All matrices/vectors are caller-owned so their original stack slots remain stable.
static inline void applyBreathYawPreservingPosition(char* matrix, char* translation, PlayerAffineScratch& yawMatrix,
                                                    UnitVec& savedPosition, float (&zeroVector)[3]) {
    __as__7WVectorFRC7WVector(translation, zeroVector);
    Matrix_ConcatAffineAssign(matrix, yawMatrix);
    __as__7WVectorFRC7WVector(translation, &savedPosition);
}

// Apply pose/animation changes to the body, club, lighting, head icon, and attachment matrices.
void CGolfPlayer::UpdatePlayerModelForDisplay(int index) {
    // Phase 1: require loaded/enabled slot and refresh its breath attachment.
    // Requires a concrete model/animation index. The later pose-only fallback is
    // historical; it does not make -1 a valid input to this display function.
    // Packed affine scratch is 3 basis vectors plus translation (12 floats).
    // Translation starts at +0x24 in the animation model matrix, +0x30 in the
    // terrain-contact frame; these are different layouts, not interchangeable.
    PlayerAffineScratch breathYawMatrix, clubWorldMatrix, correctedFootMatrix, footBasisMatrix, correctedHeadMatrix,
        headBasisMatrix;
    PlayerAffineScratch rotatedFootBasis, rotatedFootWorld, footSlopeRotation, unrotatedFootWorld;
    PlayerAffineScratch rotatedHeadBasis, rotatedHeadWorld, headYawRotation, unrotatedHeadWorld;
    UnitVec savedBreathPositionWords, headIconWorldPositionWords;
    char* contactFrameBytes;
    char* animationRecordBytes;
    char* modelOwner;    // borrowed body-model owner throughout all display phases
    char* clubBoneEntry; // separate receiver used only by phase 5
    char* transformWorkBytes;
    int modelPoolByteOffset = index * kPlayerModelStride;
    if (!(void*)ResourceModel801B_HasModelAndParts((char*)playerModels + modelPoolByteOffset))
        return;
    int slot = index;
    if (slot < 0)
        slot = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
    if (!((PlayerPoseState*)playerStates)[slot].enabled)
        return;
    animationRecordBytes = (char*)((PlayerAnimationRecord*)animationStates + index);
    modelOwner = ((char*)playerModels + modelPoolByteOffset);
    if (SharedDoc_GetCourseId() == 8) {
        char* breathBoneName = golfPlayerStrings + kTextBreathe;
        void* bodyPuppet = ((void* (*)(void*, void*))PlayerModel_GetPuppet)(modelOwner, breathBoneName);
        void* breathBoneEntry = WPuppet_FindNamedBoneEntry(bodyPuppet, (const char*)(breathBoneName));
        if (breathBoneEntry) {
            Matrix_SetAxisRotation(breathYawMatrix, kBreathEffectYaw_GolfPlayer, 1);
            int breathMatrixByteOffset = index * 0x30;
            // Keep byte-offset addition order while selecting typed packed-matrix storage.
            char* breathMatrixBytes = (char*)&((PlayerBreathMatricesView*)this)->matrices[0] + breathMatrixByteOffset;
            Matrix_CopyAffine(breathMatrixBytes, (char*)*(void**)breathBoneEntry + 0x78);
            char* translationBytes =
                (char*)&((PlayerBreathMatricesView*)this)->matrices[0].position + breathMatrixByteOffset;
            unsigned int savedX = *(unsigned int*)translationBytes;
            unsigned int savedY = *(unsigned int*)(translationBytes + 4);
            savedBreathPositionWords.x = savedX;
            savedBreathPositionWords.y = savedY;
            savedBreathPositionWords.z = *(unsigned int*)(translationBytes + 8);
            float zero[3];
            zero[0] = kFloat_0_GolfPlayer;
            zero[1] = kFloat_0_GolfPlayer;
            zero[2] = kFloat_0_GolfPlayer;
            // Apply breath yaw to axes only: remove translation, compose, restore position.
            applyBreathYawPreservingPosition(breathMatrixBytes, translationBytes, breathYawMatrix,
                                             savedBreathPositionWords, zero);
        }
    }
    // Phase 2: rebuild the body model's world basis when transform-dirty. Scripted poses
    // keep world-up, other modes use terrain contact and default/club placement.
    char* state = animationRecordBytes;
    if (((PlayerAnimationRecord*)state)->flags & kAnimationTransformDirty) {
        transformWorkBytes = (char*)((PlayerAnimationRecord*)state)->modelMatrix;
        int active = index;
        if (active < 0)
            active = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
        contactFrameBytes = (char*)((PlayerPoseState*)playerStates)[active].contactMatrix;
        unsigned int flags =
            (*(unsigned int*)&AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags);
        // Preserve contact forward, force world-up, and reconstruct right=up x forward.
        if (flags & kGameStateScriptedPose) {
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward),
                                      contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, forward));
            float v[3];
            v[0] = kFloat_0_GolfPlayer;
            v[1] = kFloat_1_GolfPlayer;
            v[2] = kFloat_0_GolfPlayer;
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up), v);
            char tmp[12];
            Vector_CrossProduct(tmp, transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up),
                                transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward));
            __as__7WVectorFRC7WVector(transformWorkBytes, tmp);
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position),
                                      contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, position));
            if (*(void**)(contactFrameBytes + 0x3C)) {
                char out[12];
                UnitVec copy = *(UnitVec*)(contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, position));
                GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), &copy, 0, 0, 0);
                __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position),
                                          out);
            }
        } else if (flags & kGameStateTerrainContactMask) {
            Matrix_CopyAffine(transformWorkBytes, contactFrameBytes);
            char out[12];
            UnitVec copy;
            char sum[12], m[12];
            Matrix_TransformPoint(m, (char*)this + index * 16 + 0x228, contactFrameBytes);
            __pl__FRC7WVectorRC7WVector(sum, contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, position),
                                        m);
            copy = *(UnitVec*)sum;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), &copy, 0, 0, 0);
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position), out);
        } else if (flags & kGameStateFlag4000) {
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward),
                                      contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, forward));
            float v[3];
            v[0] = kFloat_0_GolfPlayer;
            v[1] = kFloat_1_GolfPlayer;
            v[2] = kFloat_0_GolfPlayer;
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up), v);
            char tmp[12];
            Vector_CrossProduct(tmp, transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up),
                                transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward));
            __as__7WVectorFRC7WVector(transformWorkBytes, tmp);
            void* manager = InstanceTable_Get();
            int n = *(unsigned char*)((char*)*(void**)((char*)manager + 4) + 0xA);
            char* base = (char*)this + index * 64;
            char out[12];
            UnitVec copy;
            char sum[12], m[12];
            ((void (*)(void*, void*, void*, void*))Matrix_TransformPoint)(m, base + n * 16 + 0x128, contactFrameBytes,
                                                                          base);
            __pl__FRC7WVectorRC7WVector(sum, contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, position),
                                        m);
            copy = *(UnitVec*)sum;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(out, CQuadTree_GetInstance(), &copy, 0, 0, 0);
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position), out);
        } else {
            Matrix_CopyAffine(transformWorkBytes, contactFrameBytes);
            void* manager = InstanceTable_Get();
            int n = *(unsigned char*)((char*)*(void**)((char*)manager + 4) + 0xA);
            char* base = (char*)this + index * 64;
            char sum[12], m[12];
            ((void (*)(void*, void*, void*, void*))Matrix_TransformPoint)(m, base + n * 16 + 0x128, contactFrameBytes,
                                                                          base);
            __pl__FRC7WVectorRC7WVector(sum, contactFrameBytes + (int)DECOMP_OFFSETOF(PlayerContactFrameView, position),
                                        m);
            __as__7WVectorFRC7WVector(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position), sum);
        }
        void* parsed = ((CQuadTree*)CQuadTree_GetInstance())
                           ->GetTextureName(transformWorkBytes + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position));
        if (parsed && (!String_CompareCString(parsed, golfPlayerStrings + kTextIgrass05Dds) ||
                       !String_CompareCString(parsed, golfPlayerStrings + kTextIgrass0401Dds)))
            *(float*)(transformWorkBytes + 0x28) -= kGrassAttachmentDepth_GolfPlayer;
    }
    // Phase 3: sample animation and apply the world matrix, then update foot/head
    // bone overrides through the fixed basis and its inverse. Feet rotate about X
    // by slope; the head rotates about Y by the stored head angle (radians).
    if (((PlayerAnimationRecord*)state)->flags & 3) {
        void* p = ObjectList_GetSelected(modelOwner);
        if (p) {
            if (!*(void**)((char*)&animationDisabled)) {
                WPuppet_SetAnimationTimeFromRanges(p, 0, ((PlayerAnimationRecord*)state)->time, 0, 0);
                Model_SetMatrixRecursive(PlayerModel_GetPuppet(modelOwner), p,
                                         (char*)((PlayerAnimationRecord*)state)->modelMatrix);
            }
            WPuppet_UpdateBound(PlayerModel_GetPuppet(modelOwner), 1);
        }
    }
    if (((PlayerAnimationRecord*)state)->flags &
        (kAnimationTransformDirty | kAnimationPlaying | kAnimationAttachmentDirty)) {
        int active = index;
        if (active < 0)
            active = AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer;
        char* entry;
        char* list = (char*)((void*)playerStates) + active * kPlayerPoseStride + 0x58;
        float epsilon = kGolfPlayerNearZeroTolerance;
        entry = list;
        for (int i = 0; i < 2; i++, entry += 4)
            if (*(void**)(entry + 4)) {
                if (!*(void**)(entry + 4))
                    Matrix_SetIdentityAffine(footBasisMatrix);
                else if ((((*(unsigned int*)list & 0x80000000) ? -*(float*)list : *(float*)list) < epsilon) & 1) {
                    Matrix_ConcatAffine(unrotatedFootWorld, golfPlayerBasisMatrix, (char*)*(void**)(entry + 4) + 0x78);
                    Matrix_CopyAffine(footBasisMatrix, unrotatedFootWorld);
                } else {
                    // Foot correction: inverseBasis * (rotationX(slope) * basis * boneWorld).
                    Matrix_SetAxisRotation(footSlopeRotation, *(float*)list, 0);
                    Matrix_CopyAffine(footBasisMatrix, footSlopeRotation);
                    Matrix_ConcatAffine(rotatedFootBasis, footBasisMatrix, golfPlayerBasisMatrix);
                    Matrix_ConcatAffine(rotatedFootWorld, rotatedFootBasis, (char*)*(void**)(entry + 4) + 0x78);
                    Matrix_CopyAffine(footBasisMatrix, rotatedFootWorld);
                }
                Matrix_ConcatAffine(correctedFootMatrix, golfPlayerInverseBasisMatrix, footBasisMatrix);
                WPuppetBone_SetCurrentMatrix(*(void**)(entry + 4), correctedFootMatrix, 0);
            }
    }
    if (((PlayerAnimationRecord*)state)->flags &
        (kAnimationTransformDirty | kAnimationPlaying | kAnimationPositionDirty)) {
        int active = index;
        if (active < 0)
            active = AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer;
        transformWorkBytes = (char*)((void*)playerStates) + active * kPlayerPoseStride;
        char* headPoseBytes = transformWorkBytes;
        void* headBone = ((PlayerPoseState*)headPoseBytes)->headBone;
        if (headBone) {
            if (!headBone)
                Matrix_SetIdentityAffine(headBasisMatrix);
            else if ((((*(unsigned int*)&((PlayerPoseState*)headPoseBytes)->headRotation & 0x80000000)
                           ? -((PlayerPoseState*)headPoseBytes)->headRotation
                           : ((PlayerPoseState*)headPoseBytes)->headRotation) < kGolfPlayerNearZeroTolerance) &
                     1) {
                Matrix_ConcatAffine(unrotatedHeadWorld, golfPlayerBasisMatrix,
                                    (char*)&((PuppetBoneView*)headBone)->worldMatrix);
                Matrix_CopyAffine(headBasisMatrix, unrotatedHeadWorld);
            } else {
                // Head correction uses the same basis conjugation with a Y rotation.
                Matrix_SetAxisRotation(headYawRotation, ((PlayerPoseState*)headPoseBytes)->headRotation, 1);
                Matrix_CopyAffine(headBasisMatrix, headYawRotation);
                Matrix_ConcatAffine(rotatedHeadBasis, headBasisMatrix, golfPlayerBasisMatrix);
                Matrix_ConcatAffine(rotatedHeadWorld, rotatedHeadBasis,
                                    (char*)((PlayerPoseState*)headPoseBytes)->headBone + 0x78);
                Matrix_CopyAffine(headBasisMatrix, rotatedHeadWorld);
            }
            Matrix_ConcatAffine(correctedHeadMatrix, golfPlayerInverseBasisMatrix, headBasisMatrix);
            WPuppetBone_SetCurrentMatrix(headBone, correctedHeadMatrix, 0);
        }
    }
    // Phase 4: project the head's WORLD point, refresh lighting and compute byte
    // opacity = baseAlpha * animationOpacity. Translucency/blocked placement use 64.
    if (g_headIcon) {
        void* a = WPuppet_FindOrCacheBone(PlayerModel_GetPuppet(modelOwner), golfPlayerStrings + kTextBip01Head);
        unsigned int x = *(unsigned int*)&((PuppetBoneView*)a)->worldMatrix.position.x,
                     y = *(unsigned int*)&((PuppetBoneView*)a)->worldMatrix.position.y;
        headIconWorldPositionWords.x = x;
        headIconWorldPositionWords.y = y;
        headIconWorldPositionWords.z = *(unsigned int*)&((PuppetBoneView*)a)->worldMatrix.position.z;
        ((CHeadIcon*)g_headIcon)->ProjectPlayerPoint(index, &headIconWorldPositionWords);
    }
    if (((PlayerAnimationRecord*)state)->flags & 3)
        *(volatile unsigned int*)&((PlayerAnimationRecord*)state)->flags |=
            (kAnimationBodyLightDirty | kAnimationClubLightDirty);
    void* anim = ClubModel_FindCachedForItem(InstanceTable_Get(),
                                             (void*)((PlayerEquipmentSlotView*)g_sharedDoc)[index].clubItemId);
    void* manager = InstanceTable_Get();
    int layer = *(unsigned char*)((char*)*(void**)((char*)manager + 4) + 0xA);
    void* target = ((PlayerClubPuppetBundleView*)anim)->categoryPuppets[layer];
    if (WView_DefaultCapabilityTrueA(g_view, (char*)PlayerModel_GetPuppet(modelOwner) + 0x124)) {
        unsigned int f = ((PlayerAnimationRecord*)state)->flags;
        if (f & kAnimationBodyLightDirty) {
            if (f & kAnimationSamplePose) {
                int active = index;
                if (active < 0)
                    active = AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer;
                if (active >= 0 && active < 4) {
                    void* d = PlayerModel_GetPuppet((char*)playerModels + active * kPlayerModelStride);
                    void* x = WPuppet_FindBoneByName(d, golfPlayerStrings + kTextHead);
                    if (x) {
                        float gain = ((CQuadTree*)CQuadTree_GetInstance())
                                         ->DefaultLightingFactor(&((PuppetBoneView*)x)->worldMatrix.position);
                        *(void**)((char*)&lightColor) =
                            (void*)Color_ScaleRGB((unsigned int)(*(void**)(s_sceneLighting + 0x10)), gain);
                    }
                }
            }
            ((PlayerAnimationRecord*)state)->flags &= ~kAnimationBodyLightDirty;
            PlayerModel_ApplyBoneRelativeLighting(modelOwner, (char*)&lightType, 0, 0);
        }
        float alpha = kOpaquePlayerAlpha_GolfPlayer;
        char* config = (char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if ((*(unsigned int*)&AsCGolfDoc(config)->gameStateFlags) & kGameStateScriptedPose) {
            alpha = alpha;
        } else if (*(unsigned char*)((char*)&translucencyEnabled))
            alpha = kFloat64_GolfPlayer;
        else if (((SwingObstructionView*)this)->swingPlacementBlocked) {
            alpha = kFloat64_GolfPlayer;
            if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(config)->currentPlayer) &&
                ((*(unsigned int*)&AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->gameStateFlags) & 0xC0))
                alpha = kOpaquePlayerAlpha_GolfPlayer;
        }
        int opacity = (int)(alpha * ((PlayerAnimationRecord*)state)->opacity);
        PuppetOwner_SetOpacityRecursive(modelOwner, (unsigned char)opacity);
        WPuppet_SetOpacityRecursive(target, (unsigned char)opacity);
        if (!((((*(unsigned int*)&((PlayerAnimationRecord*)state)->opacity & 0x80000000)
                    ? -((PlayerAnimationRecord*)state)->opacity
                    : ((PlayerAnimationRecord*)state)->opacity) < kGolfPlayerNearZeroTolerance) &
              1))
            WPuppet_GetBoneFlags(target, 0);
        if (((PlayerAnimationRecord*)state)->animationSource &&
            ((PlayerAnimationRecord*)state)->animationSource !=
                ((PlayerAnimationRecord*)state)->appliedAnimationSource) {
            ((PlayerAnimationRecord*)state)->appliedAnimationSource = ((PlayerAnimationRecord*)state)->animationSource;
            AvatarModel_DefaultReturnOneHook(modelOwner);
        }
    }
    // Phase 5: reconstruct the club's world transform from the locked axes or
    // named bone attachment, add its first-axis offset, then apply character scale.
    if (!(((PlayerAnimationRecord*)state)->flags & kAnimationClubHidden)) {
        float scale = kFloat_1_GolfPlayer;
        void* m = ((CGolfDoc*)((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)))
                      ->GetItemSelection((unsigned char)((unsigned char)index));
        IFF_STRUCT::sChar* q =
            (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, *(void**)m);
        if (q)
            scale = q->scale;
        Matrix_SetIdentityAffine(clubWorldMatrix);
        if (((PlayerAnimationRecord*)state)->flags & kAnimationUseLockedAttachment) {
            char x[12], y[12], z[12];
            __ml__FRC7WVectorf(x, (char*)&attachmentDirection, scale);
            __as__7WVectorFRC7WVector(clubWorldMatrix, x);
            __ml__FRC7WVectorf(y, (char*)&((PlayerAttachmentAxesView*)this)->axisY, scale);
            __as__7WVectorFRC7WVector(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up), y);
            __ml__FRC7WVectorf(z, (char*)&((PlayerAttachmentAxesView*)this)->axisZ, scale);
            __as__7WVectorFRC7WVector(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward), z);
            __as__7WVectorFRC7WVector(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position),
                                      (char*)&attachmentPosition);
        } else {
            char* bodyModelOwner = modelOwner;
            clubBoneEntry = golfPlayerStrings + kTextClub;
            clubBoneEntry =
                (char*)WPuppet_FindNamedBoneEntry(PlayerModel_GetPuppet(bodyModelOwner), (const char*)clubBoneEntry);
            void* a = clubBoneEntry;
            if (a) {
                Matrix_CopyAffine(clubWorldMatrix, (char*)*(void**)a + 0x78);
                char translation[12];
                Matrix_TransformPoint(translation, (char*)*((void**)a + 1) + 0x18, (char*)*(void**)a + 0x78);
                __as__7WVectorFRC7WVector(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position),
                                          translation);
                void* mgr = InstanceTable_Get();
                int layer = *(unsigned char*)((char*)*(void**)((char*)mgr + 4) + 0xA);
                char* layerBase = (char*)this + index * 64;
                char* point = layerBase + layer * 16;
                char offset[12];
                ((void (*)(void*, void*, float, void*, void*))__ml__FfRC7WVector)(
                    offset, clubWorldMatrix, *(float*)(point + 0x134), point, layerBase);
                __apl__7WVectorFRC7WVector(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, position),
                                           offset);
                __amu__7WVectorFf(clubWorldMatrix, scale);
                __amu__7WVectorFf(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, up), scale);
                __amu__7WVectorFf(clubWorldMatrix + (int)DECOMP_OFFSETOF(PlayerAffineFrameView, forward), scale);
            }
        }
        Model_SetMatrixRecursive(target, 0, clubWorldMatrix);
        WPuppet_UpdateBoneWorldMatrices(target, 0, 0, kFloat_1_GolfPlayer);
        WPuppet_UpdateBoundsFromBone(target);
        if (*(unsigned int*)((char*)InstanceTable_Get() + 0x4C) & 2) {
            ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(g_shadowManager,
                                                                  (const char*)(golfPlayerStrings + kTextClub), target);
            *(void**)((char*)&clubPuppet) = target;
        }
        unsigned int f = ((PlayerAnimationRecord*)state)->flags;
        if (f & kAnimationClubLightDirty) {
            if (f & kAnimationSamplePose) {
                if (index < 0)
                    index = AsCGolfDoc((*(void**)&g_sharedDoc->golfDoc))->currentPlayer;
                if (index >= 0 && index < 4) {
                    void* d = PlayerModel_GetPuppet((char*)playerModels + index * kPlayerModelStride);
                    void* x = WPuppet_FindBoneByName(d, golfPlayerStrings + kTextHead);
                    if (x) {
                        float gain = ((CQuadTree*)CQuadTree_GetInstance())
                                         ->DefaultLightingFactor(&((PuppetBoneView*)x)->worldMatrix.position);
                        *(void**)((char*)&lightColor) =
                            (void*)Color_ScaleRGB((unsigned int)(*(void**)(s_sceneLighting + 0x10)), gain);
                    }
                }
            }
            ((PlayerAnimationRecord*)state)->flags &= ~kAnimationClubLightDirty;
            ((UnitObject*)target)->update((char*)&lightType, 0, 0);
        } else {
            ((UnitObject*)target)->update((char*)&lightType, 0, 0);
        }
        DrawScene_SetNamedObjectPuppet(g_sceneManager, (const char*)(golfPlayerStrings + kTextClubActor), target);
    }
    // Consume transform/attachment/position dirtiness; keep playing and other flags.
    ((PlayerAnimationRecord*)state)->flags &=
        ~(kAnimationTransformDirty | kAnimationAttachmentDirty | kAnimationPositionDirty);
}

// Set the active puppet's animation-disabled state, returning zero when it has no model.
int CGolfPlayer::SetAnimationDisabled(int disabled) {
    void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerModels = this->playerModels;
    void* puppet = PlayerModel_GetPuppet((char*)playerModels + playerIndex * kPlayerModelStride);
    if (puppet == 0)
        return 0;
    *(int*)((char*)&animationDisabled) = disabled;
    return WPuppet_SetAnimationDisabled(puppet, disabled != 0);
}

// Allocate and initialize the five transform buffers used by the animation blender.
int CGolfPlayer::AllocateTransformBuffers(void** transformBuffers) {
    // Caller passes five initialized pointer slots (null or prior new[] buffers).
    // Replaces each with FtCount * 0x30 bytes; return 0 may leave a partially
    // replaced array, still owned by caller and releasable with Array_FreeFiveEntries.
    int transformBytes;
    int bufferIndex;
    void* puppet;
    void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerModels = this->playerModels;
    puppet = PlayerModel_GetPuppet((char*)playerModels + playerIndex * kPlayerModelStride);
    transformBytes = WPuppet_GetFtCount(puppet) * 0x30;
    bufferIndex = 0;
    while (bufferIndex < kTransformBufferCount) {
        if (transformBuffers[bufferIndex] != 0) {
            __dla__FPv(transformBuffers[bufferIndex]);
            transformBuffers[bufferIndex] = 0;
        }
        transformBuffers[bufferIndex] = __nwa__FUl((unsigned int)(transformBytes));
        if (transformBuffers[bufferIndex] == 0)
            return 0;
        WPuppet_EvaluateModelTransformAnimation(puppet, *(void* volatile*)&transformBuffers[bufferIndex]);
        bufferIndex++;
    }
    return 1;
}

// Release and clear all five transform buffers.
int Array_FreeFiveEntries(void* unusedOwner, void** transformBuffers) {
    int bufferIndex = 0;
    while (bufferIndex < kTransformBufferCount) {
        if (transformBuffers[bufferIndex] != 0) {
            __dla__FPv(transformBuffers[bufferIndex]);
            transformBuffers[bufferIndex] = 0;
        }
        bufferIndex++;
    }
    return 1;
}

// Evaluate one named animation and replace the active model's skin matrices.
int CGolfPlayer::OverrideSkinWithNamedAnimation(void* animationName, float animationTime) {
    // Return 1 only after a successful skin override; caller retains animationName.
    // Failure behavior is preserved: a missing range leaves boneTransforms
    // allocated, and failed animation evaluation leaves nameBuffer allocated.
    // Phase 1: copy base transforms and resolve/copy the selected track data.
    void* boneTransforms;
    void* puppet;
    void* animationPuppet;
    int boneTransformCount;
    void* animationRange;
    unsigned short nameIndex;
    int nameBytes;
    void* nameBuffer;
    unsigned short copiedNameIndex;
    int skinMatrixCount;
    void* skinMatrices;
    puppet = PlayerModel_GetPuppet((char*)playerModels +
                                   AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer *
                                       kPlayerModelStride);
    animationPuppet = ObjectList_GetSelected((char*)playerModels +
                                             AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer *
                                                 kPlayerModelStride);
    if (puppet == 0)
        return 0;
    boneTransformCount = WPuppet_GetFtCount(puppet);
    if (boneTransformCount == 0)
        return 0;
    boneTransforms = __nwa__FUl((unsigned int)(boneTransformCount * 0x30));
    if (boneTransforms == 0)
        return 0;
    WPuppet_EvaluateModelTransformAnimation(puppet, boneTransforms);
    animationRange = (void*)WPuppet_FindAnimationRangeByName(animationPuppet, animationName);
    if (animationRange == 0)
        return 0;
    nameIndex = ((PlayerAnimationRange*)animationRange)->animationNameIndex;
    nameBytes = WPuppet_GetAnimationNameSize(animationPuppet, nameIndex);
    nameBuffer = __nwa__FUl((unsigned int)(nameBytes));
    copiedNameIndex = ((PlayerAnimationRange*)animationRange)->animationNameIndex;
    WPuppet_CopyAnimationName(animationPuppet, nameBuffer, copiedNameIndex, nameBytes);
    if (!WPuppet_EvaluateAnimationTransforms(animationPuppet, boneTransforms, boneTransformCount, nameBuffer,
                                             animationTime)) {
        if (boneTransforms != 0)
            __dla__FPv(boneTransforms);
        return 0;
    }
    __dla__FPv(nameBuffer);
    // Phase 2: evaluate skin matrices from sampled transforms, override the puppet,
    // and release successful-path temporary arrays after the override call.
    skinMatrixCount = WPuppet_GetSnrtCount(puppet);
    if (skinMatrixCount == 0) {
        if (boneTransforms != 0)
            __dla__FPv(boneTransforms);
        return 0;
    }
    skinMatrices = __nwa__FUl((unsigned int)(skinMatrixCount << 6));
    if (skinMatrices == 0) {
        if (boneTransforms != 0)
            __dla__FPv(boneTransforms);
        return 0;
    }
    char temp[0x40];
    PSMTX44Identity(temp);
    if (!WPuppet_EvaluateTransforms(puppet, skinMatrices, skinMatrixCount, temp, boneTransforms, boneTransformCount)) {
        if (boneTransforms != 0)
            __dla__FPv(boneTransforms);
        if (skinMatrices != 0)
            __dla__FPv(skinMatrices);
        return 0;
    }
    if (!WPuppet_OverrideSkinMatricesForwarder(puppet, skinMatrices, skinMatrixCount)) {
        if (boneTransforms != 0)
            __dla__FPv(boneTransforms);
        if (skinMatrices != 0)
            __dla__FPv(skinMatrices);
        return 0;
    }
    if (boneTransforms != 0)
        __dla__FPv(boneTransforms);
    if (skinMatrices != 0)
        __dla__FPv(skinMatrices);
    return 1;
}

// Sample and blend two named animation tracks, then replace the model's skin matrices.
int CGolfPlayer::BlendAnimationTracksAndApplyPose(float horizontalTime, float verticalTime, float blendWeight,
                                                  void* horizontalAnimation, void* verticalAnimation,
                                                  void* blendBuffers) {
    // blendBuffers borrows the five initialized buffers made by AllocateTransformBuffers.
    // For nonnegative track limits, clamp each time to [0,limit] and sample
    // floor(t)/ceil(t), interpolating by frac(t). blendWeight is not clamped.
    // Return 0 on a missing track/model or evaluation/allocation failure.
    // Phase 1: resolve limits/model and choose adjacent sample coordinates.
    float horizontalEnd = kFloat_0_GolfPlayer;
    float verticalEnd = kFloat_0_GolfPlayer;
    void* skinMatrices;
    unsigned int offset;
    int j;
    int i;
    if (horizontalTime < kFloat_0_GolfPlayer)
        horizontalTime = kFloat_0_GolfPlayer;
    if (verticalTime < kFloat_0_GolfPlayer)
        verticalTime = kFloat_0_GolfPlayer;
    if (!((LimitObj*)this)->getLimit(&horizontalEnd, horizontalAnimation))
        return 0;
    if (!((LimitObj*)this)->getLimit(&verticalEnd, verticalAnimation))
        return 0;
    if (horizontalTime > horizontalEnd)
        horizontalTime = horizontalEnd;
    if (verticalTime > verticalEnd)
        verticalTime = verticalEnd;
    void* puppet = PlayerModel_GetPuppet((char*)playerModels +
                                         AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer *
                                             kPlayerModelStride);
    void* animationPuppet = ObjectList_GetSelected(
        (char*)playerModels +
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer * kPlayerModelStride);
    if (!puppet)
        return 0;
    int xi = (int)horizontalTime;
    int ix2 = xi + 1;
    int yi = (int)verticalTime;
    int iy2 = yi + 1;
    float fx = horizontalTime - (float)xi;
    float fy = verticalTime - (float)yi;
    if (kFloat_0_GolfPlayer == fx)
        ix2 = xi;
    if (kFloat_0_GolfPlayer == fy)
        iy2 = yi;
    int count = WPuppet_GetFtCount(puppet);
    if (count == 0)
        return 0;
    int ok = 1;
    // Phase 2: sample/interpolate the horizontal track's rotation and translation.
    void* horizontalRange = (void*)WPuppet_FindAnimationRangeByName(animationPuppet, horizontalAnimation);
    if (horizontalRange == 0)
        return 0;
    unsigned short id = ((PlayerAnimationRange*)horizontalRange)->animationNameIndex;
    int n = WPuppet_GetAnimationNameSize(animationPuppet, id);
    void* buffer = __nwa__FUl((unsigned int)(n));
    unsigned short id2 = ((PlayerAnimationRange*)horizontalRange)->animationNameIndex;
    WPuppet_CopyAnimationName(animationPuppet, buffer, id2, n);
    if (WPuppet_EvaluateAnimationTransforms(animationPuppet, ((AnimationBlendBuffers*)blendBuffers)->firstSample, count,
                                            buffer, (float)xi) == 0)
        ok = 0;
    if (WPuppet_EvaluateAnimationTransforms(animationPuppet, ((AnimationBlendBuffers*)blendBuffers)->secondSample,
                                            count, buffer, (float)ix2) == 0)
        ok = 0;
    __dla__FPv(buffer);
    if (ok == 0)
        return 0;
    for (i = 0, offset = 0; i < (unsigned int)count; offset += 0x30, ++i) {
        Vec3_Lerp((char*)((AnimationBlendBuffers*)blendBuffers)->horizontal + offset + 0x10,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->firstSample + offset + 0x10,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->secondSample + offset + 0x10, fx);
        Vec3_Lerp((char*)((AnimationBlendBuffers*)blendBuffers)->horizontal + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->firstSample + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->secondSample + offset + 0x20, fx);
    }
    // Phase 3: reuse the sample buffers for the vertical track, retaining horizontal.
    void* verticalRange = (void*)WPuppet_FindAnimationRangeByName(animationPuppet, verticalAnimation);
    if (!verticalRange)
        return 0;
    unsigned short id3 = ((PlayerAnimationRange*)verticalRange)->animationNameIndex;
    n = WPuppet_GetAnimationNameSize(animationPuppet, id3);
    void* buffer2 = __nwa__FUl((unsigned int)(n));
    unsigned short id4 = ((PlayerAnimationRange*)verticalRange)->animationNameIndex;
    WPuppet_CopyAnimationName(animationPuppet, buffer2, id4, n);
    if (WPuppet_EvaluateAnimationTransforms(animationPuppet, ((AnimationBlendBuffers*)blendBuffers)->firstSample, count,
                                            buffer2, (float)yi) == 0)
        ok = 0;
    if (WPuppet_EvaluateAnimationTransforms(animationPuppet, ((AnimationBlendBuffers*)blendBuffers)->secondSample,
                                            count, buffer2, (float)iy2) == 0)
        ok = 0;
    __dla__FPv(buffer2);
    if (ok == 0)
        return 0;
    for (j = 0, offset = 0; j < (unsigned int)count; offset += 0x30, ++j) {
        Vec3_Lerp((char*)((AnimationBlendBuffers*)blendBuffers)->vertical + offset + 0x10,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->firstSample + offset + 0x10,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->secondSample + offset + 0x10, fy);
        Vec3_Lerp((char*)((AnimationBlendBuffers*)blendBuffers)->vertical + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->firstSample + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->secondSample + offset + 0x20, fy);
    }
    for (j = 0, offset = 0; j < (unsigned int)count; offset += 0x30, ++j) {
        // Phase 4: blend the tracks with wrapped Euler angles; retain sample scale.
        ((int (*)(void*, void*, void*, void*, float))EulerAngles_LerpWrapped)(
            this, (char*)((AnimationBlendBuffers*)blendBuffers)->blended + offset + 0x10,
            (char*)((AnimationBlendBuffers*)blendBuffers)->horizontal + offset + 0x10,
            (char*)((AnimationBlendBuffers*)blendBuffers)->vertical + offset + 0x10, blendWeight);
        Vec3_Lerp((char*)((AnimationBlendBuffers*)blendBuffers)->blended + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->horizontal + offset + 0x20,
                  (char*)((AnimationBlendBuffers*)blendBuffers)->vertical + offset + 0x20, blendWeight);
        Vector_CopyComponents((char*)((AnimationBlendBuffers*)blendBuffers)->blended + offset,
                              (char*)((AnimationBlendBuffers*)blendBuffers)->firstSample + offset);
    }
    // Phase 5: evaluate the blended transforms into skin matrices, install them
    // and free only the temporary skin allocation; all five work buffers stay borrowed.
    int animCount = WPuppet_GetSnrtCount(puppet);
    if (!animCount)
        return 0;
    skinMatrices = __nwa__FUl((unsigned int)(animCount << 6));
    if (!skinMatrices)
        return 0;
    char temp[0x40];
    PSMTX44Identity(temp);
    if (!WPuppet_EvaluateTransforms(puppet, skinMatrices, animCount, temp,
                                    ((AnimationBlendBuffers*)blendBuffers)->blended, count)) {
        if (skinMatrices != 0)
            __dla__FPv(skinMatrices);
        return 0;
    }
    if (!WPuppet_OverrideSkinMatricesForwarder(puppet, skinMatrices, animCount)) {
        if (skinMatrices != 0)
            __dla__FPv(skinMatrices);
        return 0;
    }
    if (skinMatrices != 0)
        __dla__FPv(skinMatrices);
    return 1;
}

// Resolve a range's animation name and query its value into the supplied output.
int CGolfPlayer::GetAnimationValueForRange(void* valueOutput, void* animationName) {
    void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char index = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerModels = this->playerModels;
    void* animationPuppet = ObjectList_GetSelected((char*)playerModels + index * kPlayerModelStride);
    if (animationPuppet == 0)
        return 0;
    void* animationRange = (void*)WPuppet_FindAnimationRangeByName(animationPuppet, animationName);
    if (animationRange == 0)
        return 0;
    unsigned short id = ((PlayerAnimationRange*)animationRange)->animationNameIndex;
    int nameBytes = WPuppet_GetAnimationNameSize(animationPuppet, id);
    void* nameBuffer = __nwa__FUl((unsigned int)(nameBytes));
    unsigned short id2 = ((PlayerAnimationRange*)animationRange)->animationNameIndex;
    WPuppet_CopyAnimationName(animationPuppet, nameBuffer, id2, nameBytes);
    void* result = WPuppet_GetAnimationValueByName(animationPuppet, valueOutput, nameBuffer);
    __dla__FPv(nameBuffer);
    return (int)result;
}
}
