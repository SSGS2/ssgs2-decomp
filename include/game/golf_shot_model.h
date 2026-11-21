// Borrowed Wii runtime views established by the round/shot producers and consumers.
// Biased account pointers are SharedDoc + channel*0x1D4, not an asserted array base.
// Alternate scalar widths remain explicit where existing class headers differ.
#ifndef GAME_GOLF_SHOT_MODEL_H
#define GAME_GOLF_SHOT_MODEL_H

#include "game/golfball.h"
#include "classes/CGolfRule.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CSharedDoc.h"

typedef sExpected ShotSample;

// Compatibility spelling for the canonical shared rule layout.
typedef CGolfRule GolfRuleFrameFields;
#define RULE_FRAME_OFF(f, o) typedef char RuleFrame_##f[((int)&((GolfRuleFrameFields*)0)->f == (o)) ? 1 : -1]
RULE_FRAME_OFF(elapsedTime, 0x34);
RULE_FRAME_OFF(animationStarted, 0x74);
RULE_FRAME_OFF(aimTarget, 0x78);
RULE_FRAME_OFF(stateEntryPending, 0x90);
RULE_FRAME_OFF(stateTimer, 0x9C);
RULE_FRAME_OFF(courseCameraBounds, 0xA0);
RULE_FRAME_OFF(doublePowerCameraReset, 0xA4);
RULE_FRAME_OFF(launchHeadingOffset, 0xA8);
RULE_FRAME_OFF(clubName, 0x35C);
RULE_FRAME_OFF(ballFlightSoundName, 0x374);
RULE_FRAME_OFF(networkWaitTimeout, 0x3C8);
RULE_FRAME_OFF(playbackFrame, 0x478);
RULE_FRAME_OFF(simulationStartFrame, 0x474);
RULE_FRAME_OFF(shotContactFrame, 0x47C);
RULE_FRAME_OFF(shotPresentationFlags, 0x490);
RULE_FRAME_OFF(presentationStep, 0x494);
RULE_FRAME_OFF(tutorialFeedbackEnabled, 0x4BC);
RULE_FRAME_OFF(tutorialAdvancePending, 0x4C0);
RULE_FRAME_OFF(fastPlayback, 0x4E4);
RULE_FRAME_OFF(courseReloadStage, 0x4D0);
RULE_FRAME_OFF(holeSpawnPosition, 0x4E8);
#undef RULE_FRAME_OFF


// Live ball +A4 is float simulation time, overlapping the legacy event-array tail.
// Golf document +2D8 is read as a byte in both launch pipelines; meaning unknown.
typedef CGolfBall BallShotTimingView;
struct GolfDocShotBytes { char prefix[0x2D8]; unsigned char unk_2D8; };
typedef char BallShotTimingOffset[(int)&((BallShotTimingView*)0)->simulationSeconds == 0xA4 ? 1 : -1];
typedef char GolfDocShotByteOffset[(int)&((GolfDocShotBytes*)0)->unk_2D8 == 0x2D8 ? 1 : -1];
// Button input tick fields: OS_GetTimeMilliseconds is stored in signed words;
// subtraction is converted to unsigned elapsed milliseconds before float scaling.
// +40C takes the minimum nonzero deltaSeconds during power selection.
typedef CGolfRule RuleGaugeInputTiming;
typedef char GaugeTickOffset[(int)&((RuleGaugeInputTiming*)0)->lastGaugeTick == 0x3A8 ? 1 : -1];
typedef char GaugeFirstTickOffset[(int)&((RuleGaugeInputTiming*)0)->firstGaugeTick == 0x3AC ? 1 : -1];
typedef char GaugeDirectionTickOffset[(int)&((RuleGaugeInputTiming*)0)->heldDirectionTick == 0x3E4 ? 1 : -1];
typedef char GaugeMinimumFrameOffset[(int)&((RuleGaugeInputTiming*)0)->minimumPowerFrameSeconds == 0x40C ? 1 : -1];
// Shared +9C6 selects rain/lightning/ambient branches; wider meaning unknown.
// Account +1CC is passed to the club-set test; +1E0 is the motion-controller option.
typedef CSharedDoc SharedEntryProtocol;
struct AccountClubSelectionFields {
    char prefix[0x1CC]; void* clubSetValue;
    char pad_1D0[0x10]; int unk_1E0;
};
typedef char SharedEntryOffset[(int)&((SharedEntryProtocol*)0)->weatherType == 0x9C6 ? 1 : -1];
typedef char AccountClubOffset[(int)&((AccountClubSelectionFields*)0)->clubSetValue == 0x1CC ? 1 : -1];
typedef char AccountMotionOffset[(int)&((AccountClubSelectionFields*)0)->unk_1E0 == 0x1E0 ? 1 : -1];
// Common shot model (all views borrow existing storage).
// Account view is biased to SharedDoc + channel*0x1D4, NOT an asserted array base.
struct ShotWorldClockView { char prefix[0x21A8]; float fractionalSeconds; int frame; };
typedef CGolfBall BallGroundTriangleView;
struct BallHoleInWordsView { char prefix[0x1989C]; int encodedValue, addressMix, key; };
struct ShotAccountStatisticsView {
    char prefix[0x4C]; int nonPutterShots, putterShots, manualSwings, assistedPangyas;
    unsigned short holeInOnes, albatrosses;                       // +5C, +5E
    unsigned int openingShots, openingShotsOnFairwayOrGreen;      // +60, +64
    unsigned int madePutts, recoveryShots;                        // +68, +6C
    float longestHoledOrBouncedShotYards;                         // +70
    float longestMadePuttYards, longestNonPutterHoleOutYards;      // +74, +78
    unsigned int totalNonRecoveryWholeYards;                      // +7C
    float swingGaugeTotal;                                       // +80
    char gap84[0x9C]; int characterId;
};
typedef GolfDocSnapshot8013 ShotPlayerTotalsView;
struct RuleShotCheckpointView {
    char prefix[0x10C]; WVector playerPosition; float headingRadians;
    unsigned char recoveryRequired, playerByteAE; char reserved11E[2];
    float powerShotMeter; int pang, bonusPang, holePang;
    char reserved130[0x20]; char clubName[4];
    unsigned char holeStrokes, holeResult, playerByteBC; char reserved157;
    int partyResult, powerMeterCost;
};
// +0x10 counts occupied map slots; allocation capacity is the next word, +0x14.
struct ShotHistoryHeaderView { int head, count; void* blockMap[2]; int blockMapCount; };
// Checkpoint, history storage and hole record: game/golf_shot_types.h.
// Unknown bytes keep offset names: ProcessShotOutcome writes 92, tests/clears 90;
// undo clears 91/92/94; playback tests 408 and shot completion clears it.
typedef CGolfRule RuleUnresolvedShotBytes;
#define SHOT_MODEL_OFF(t, f, o) typedef char ShotModel_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
SHOT_MODEL_OFF(ShotSample, m_pos, 0x00);
SHOT_MODEL_OFF(ShotSample, velocity, 0xC);
SHOT_MODEL_OFF(ShotSample, rotation, 0x18);
SHOT_MODEL_OFF(ShotSample, eventSoundName, 0x20);
SHOT_MODEL_OFF(ShotSample, eventKind, 0x60);
SHOT_MODEL_OFF(ShotSample, eventSubkind, 0x61);
SHOT_MODEL_OFF(ShotSample, contactNormal, 0x64);
SHOT_MODEL_OFF(ShotSample, motionState, 0x70);
SHOT_MODEL_OFF(ShotAccountStatisticsView, manualSwings, 0x54);
SHOT_MODEL_OFF(ShotAccountStatisticsView, holeInOnes, 0x5C);
SHOT_MODEL_OFF(ShotAccountStatisticsView, albatrosses, 0x5E);
SHOT_MODEL_OFF(ShotAccountStatisticsView, openingShots, 0x60);
SHOT_MODEL_OFF(ShotAccountStatisticsView, openingShotsOnFairwayOrGreen, 0x64);
SHOT_MODEL_OFF(ShotAccountStatisticsView, madePutts, 0x68);
SHOT_MODEL_OFF(ShotAccountStatisticsView, recoveryShots, 0x6C);
SHOT_MODEL_OFF(ShotAccountStatisticsView, longestHoledOrBouncedShotYards, 0x70);
SHOT_MODEL_OFF(ShotAccountStatisticsView, longestMadePuttYards, 0x74);
SHOT_MODEL_OFF(ShotAccountStatisticsView, longestNonPutterHoleOutYards, 0x78);
SHOT_MODEL_OFF(ShotAccountStatisticsView, totalNonRecoveryWholeYards, 0x7C);
SHOT_MODEL_OFF(ShotAccountStatisticsView, swingGaugeTotal, 0x80);
SHOT_MODEL_OFF(ShotWorldClockView, frame, 0x21AC);
SHOT_MODEL_OFF(ShotCheckpoint, powerShotMeter, 0x14);
SHOT_MODEL_OFF(ShotCheckpoint, clubName, 0x44);
SHOT_MODEL_OFF(ShotCheckpoint, powerMeterCost, 0x50);
SHOT_MODEL_OFF(ShotHoleRecord, par, 0x18);
SHOT_MODEL_OFF(RuleUnresolvedShotBytes, trajectoryEventDispatched, 0x408);
#undef SHOT_MODEL_OFF
typedef char ShotCheckpointSize[(sizeof(ShotCheckpoint) == 0x54) ? 1 : -1];
typedef char ShotHoleRecordSize[(sizeof(ShotHoleRecord) == 0x20) ? 1 : -1];
// Event 2/4 and slots 3/4/5/9 remain numeric unknowns. Known event 1 carries
// contactNormal; event 3 carries object-hit sound; event 5 starts booster warp.
enum ShotSampleEventKind { kSampleContact = 1, kSampleUnknown2 = 2,
    kSampleObjectHit = 3, kSampleUnknown4 = 4, kSampleBoosterWarp = 5 };


// Runtime club records are distinct from the inventory's IFF sClub records.
struct SelectedClubRuntime {
    char pad_000[8];
    unsigned char specialShotVariant;
    char pad_009;
    unsigned char clubKind;
    char pad_00B[0xD];
    char clubName[4]; // +18: copied by both launch pipelines into checkpoint +44
};
struct ClubSelectionRuntime {
    char pad_000[4];
    SelectedClubRuntime* selectedClub;
    char pad_008[0x3C];
    int powerShotMode;
    unsigned char shotCommandKind;
    char pad_049[0x2F];
    int hudPhase; // +78: screen query 0x30 reply; CClub_ClubProcess's inputState
};
typedef char ClubKindOffset[((int)&((SelectedClubRuntime*)0)->clubKind == 0xA) ? 1 : -1];
typedef char ClubShotModeOffset[((int)&((ClubSelectionRuntime*)0)->powerShotMode == 0x44) ? 1 : -1];
typedef char ClubHudPhaseOffset[((int)&((ClubSelectionRuntime*)0)->hudPhase == 0x78) ? 1 : -1];
typedef char ClubShotCommandOffset[((int)&((ClubSelectionRuntime*)0)->shotCommandKind == 0x48) ? 1 : -1];

// CSharedDoc contains four 0x1D4-byte player selections. This view is biased to
// document base + channel*0x1D4; +1DC is the selected player's button-swing option
// (0 motion / nonzero buttons), as consumed by ProcessAimInput. It owns no pointer.
struct PlayerSwingInputView { char prefix[0x1DC]; int buttonSwingEnabled; };
// A member-array expression hoists an address in UpdateBallPlayback; keep this addend.
enum ShotSampleOffset { kSimulationEventSoundOffset = 0x20 };
enum ShotMode { kNormalShotMode = 0, kPowerShotMode = 1, kDoublePowerShotMode = 2 };

// Four rule-owned shot histories are constructed and destroyed as 24-byte records.
struct ShotHistoryContainer { unsigned int words[6]; };
struct RuleShotHistories { char pad_000[0xAC]; ShotHistoryContainer players[4]; };

// Sparse shared-document fields used by computer-player resource loading.
struct SharedComputerRoundFields {
    char pad_000[0x9E4];
    char profileName[0x20];
    char pad_A04[0x16C];
    int strokeScores[4];
};

// Offsets below supplement gaps in the shared ball, quadtree and task headers.
struct RuleShotCompletionBytes {
    char prefix[0x90];
    unsigned char stateEntryPending, unresolved91, unresolved92, shotUndoEnabled, unresolved94;
};
typedef char RuleCompletion94Offset[(int)&((RuleShotCompletionBytes*)0)->unresolved94 == 0x94 ? 1 : -1];
// Actor IDs below are scoped to their recipients; all payload words here are zero.
enum ShotCompletionActorMessage {
    kScreenFinalizeShotMessage17 = 0x17, kScoreFinalizeShotMessage5 = 5,
    kNpcFinalizeShotMessage0 = 0, kBallFinalizeShotMessage3 = 3,
    kScreenFinalizeShotMessage7 = 7, kSpinFinalizeShotMessage4 = 4,
    kGuidelineFinalizeShotMessage1 = 1
};

struct BallGroundContact { char pad_000[0x28]; void* triangle; };
struct QuadTreeRecoveryPoint { char pad_000[0x2084]; WVector position; };
struct AimRecoveryPoint { char pad_000[0xC]; WVector position; };
struct CourseCameraAnchor { char pad_000[0x44]; WVector position; };
struct PlayerSwingOptions { char pad_000[0x1DC]; int buttonSwingSelected; };
// Camera action 10 asks the caddie for a shot camera only while this byte is set.
struct CameraCaddieOption { char pad_000[0x1A2]; unsigned char caddieCameraEnabled; };

#endif
