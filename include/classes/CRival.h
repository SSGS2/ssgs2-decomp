// CRival: the CPU rival golfer.
//
// One instance exists for the round. It owns the shot pipeline, the shot
// position thinker, the stroke history, the shot-error randomiser, the
// unplayable-shot handler and the checkpoint rule evaluator. The class caches
// the shot gauge it prepared so the shot can be applied on a later frame.
//
// The class declares no virtual functions: its vtable is original game data, so
// `vtbl` is an explicit first member.
#ifndef CLASSES_CRIVAL_H
#define CLASSES_CRIVAL_H

#include "game/types.h"

class CRival {
public:
    // Build the rival singleton and its owned helpers. Named Init rather than a
    // constructor: callers build the rival in raw storage.
    CRival* Init();
    ~CRival();

    void PrepareShotGauge(float normalizedPower, float impactOffset);
    // The rule frame argument is part of the original call shape; the body does
    // not read it.
    int ApplyPreparedShot(void* ruleFrame);

    // Probe straight and rotated sweeps toward the target; changes shotCurve
    // when a curved sweep clears the obstruction.
    int ChooseCurveAroundObstruction(Vec3f* target);

    // Try club choices, simulate the target and fill the caller's shot
    // candidate record from the result.
    void SelectClubAndComputeShot(void* out, float headingRadians);
    // Save the club picked for this hole and restart the correction chain.
    void ResetShotThinkingState();
    // Run the shot pipeline's nested tasks. The caller's task context is unused.
    int UpdateShotProcessor(void* taskContext);
    int IsUnplayableDropActive();
    // Give the shot-position thinker the current target area and player.
    void SetShotPositionCandidateContext();

    void* GetThinkController();

    static void* GetInstance();

    static void* GetShotPositionThinker(void* object);

    void* vtbl;                    // 0x0: original vtable pointer
    char pad_004[0xc];             // 0x4
    Vec3f targetPosition;          // 0x10: the checkpoint the rival is shooting at
    float holePosition[3];         // 0x1c: current hole entry position
    char powerShotLevel;           // 0x28
    char pad_029[0x7];             // 0x29
    int shotApplyPhase;            // 0x30
    float shotPowerRatio;          // 0x34
    float shotCurve;               // 0x38
    float shotSpin;                // 0x3c
    void* shotPositionThinker;     // 0x40
    void* strokeUse;               // 0x44
    void* randomUse;               // 0x48
    void* unplayableHandler;       // 0x4c
    void* thinkController;         // 0x50
    void* currentCheckpoint;       // 0x54: the checkpoint being tried; a target-area view of it has an index at +0x1C
    float shotPowerGaugeX;         // 0x58
    float shotImpactGaugeX;        // 0x5c
    float shotGaugeCursorX;        // 0x60
    float roundedShotGaugeCursorX; // 0x64
    int impactGrade;               // 0x68
    unsigned char preparedPangyaAssistFlag; // 0x6c
    char pad_06D[0x3];                      // 0x6d
    int shotGaugeDirection;                 // 0x70
    float pangyaTargetX;                    // 0x74
    float calipersGaugeX;                   // 0x78
    float pangyaZoneScale;                  // 0x7c
    int impactTimingSpriteIndex;            // 0x80
    float impactFeedbackGaugeX;             // 0x84
    int powerShotBonusPang;                 // 0x88
    int pangyaAssistState;                  // 0x8c
    int preparedUnassistedImpactGaugeBits;  // 0x90
    int unassistedImpactGrade;              // 0x94
    char pad_098[0x4];                      // 0x98
    void* shotProcessor;                    // 0x9c
    int savedClubIndex;                     // 0xa0
    int selectedClubIndex;                  // 0xa4
    float simulationStartFrame;             // 0xa8
    char difficultyEnabled;                 // 0xac
    char pad_0AD[0x3];                      // 0xad
};

#define CRIVAL_OFF(f, o) typedef char CRival_off_##f[((int)&((CRival*)0)->f == (o)) ? 1 : -1]
CRIVAL_OFF(targetPosition, 0x10);
CRIVAL_OFF(holePosition, 0x1c);
CRIVAL_OFF(powerShotLevel, 0x28);
CRIVAL_OFF(shotCurve, 0x38);
CRIVAL_OFF(shotSpin, 0x3c);
CRIVAL_OFF(shotPositionThinker, 0x40);
CRIVAL_OFF(strokeUse, 0x44);
CRIVAL_OFF(randomUse, 0x48);
CRIVAL_OFF(unplayableHandler, 0x4c);
CRIVAL_OFF(thinkController, 0x50);
CRIVAL_OFF(currentCheckpoint, 0x54);
CRIVAL_OFF(shotProcessor, 0x9c);
CRIVAL_OFF(savedClubIndex, 0xa0);
CRIVAL_OFF(selectedClubIndex, 0xa4);
CRIVAL_OFF(difficultyEnabled, 0xac);
CRIVAL_OFF(shotApplyPhase, 0x30);
CRIVAL_OFF(shotPowerRatio, 0x34);
CRIVAL_OFF(shotPowerGaugeX, 0x58);
CRIVAL_OFF(shotImpactGaugeX, 0x5c);
CRIVAL_OFF(shotGaugeCursorX, 0x60);
CRIVAL_OFF(roundedShotGaugeCursorX, 0x64);
CRIVAL_OFF(impactGrade, 0x68);
CRIVAL_OFF(preparedPangyaAssistFlag, 0x6c);
CRIVAL_OFF(shotGaugeDirection, 0x70);
CRIVAL_OFF(pangyaTargetX, 0x74);
CRIVAL_OFF(calipersGaugeX, 0x78);
CRIVAL_OFF(pangyaZoneScale, 0x7c);
CRIVAL_OFF(impactTimingSpriteIndex, 0x80);
CRIVAL_OFF(impactFeedbackGaugeX, 0x84);
CRIVAL_OFF(powerShotBonusPang, 0x88);
CRIVAL_OFF(pangyaAssistState, 0x8c);
CRIVAL_OFF(preparedUnassistedImpactGaugeBits, 0x90);
CRIVAL_OFF(unassistedImpactGrade, 0x94);
CRIVAL_OFF(simulationStartFrame, 0xa8);
#undef CRIVAL_OFF
typedef char CRival_size[(sizeof(CRival) == 0xb0) ? 1 : -1];

#endif // CLASSES_CRIVAL_H
