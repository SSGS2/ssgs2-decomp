// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CRIVAL_H
#define CLASSES_CRIVAL_H

#include "game/types.h"

class CRival {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    float holePosition[3];             // 0x1c: 0x801DFED4 copies the current hole entry's position; CComShotProc_NextCandidateOrUseFallback resets it
    char powerShotLevel;                        // 0x28: 1 use in CRival_Ctor
    char pad_029[0x7]; // 0x29
    int shotApplyPhase; // 0x30: CRival_ApplyPreparedShot
    float shotPowerRatio; // 0x34: CRival_ApplyPreparedShot
    float shotCurve;                       // 0x38: 1 use in CRival_Ctor
    float shotSpin;                       // 0x3c: 1 use in CRival_Ctor
    void* shotPositionThinker;                       // 0x40: 3 uses in CRival_Ctor, CRival_dtor
    void* strokeUse;                       // 0x44: 2 uses in CRival_Ctor, CRival_dtor
    void* randomUse;                       // 0x48: 2 uses in CRival_Ctor, CRival_dtor
    void* unplayableHandler;                       // 0x4c: 2 uses in CRival_Ctor, CRival_dtor
    void* thinkController;                       // 0x50: 2 uses in CRival_Ctor, CRival_dtor
    char pad_054[0x4]; // 0x54
    float shotPowerGaugeX; // 0x58: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float shotImpactGaugeX; // 0x5c: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float shotGaugeCursorX; // 0x60: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float roundedShotGaugeCursorX; // 0x64: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int impactGrade; // 0x68: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    unsigned char preparedPangyaAssistFlag; // 0x6c: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    char pad_06D[0x3]; // 0x6d
    int shotGaugeDirection; // 0x70: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float pangyaTargetX; // 0x74: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float calipersGaugeX; // 0x78: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float pangyaZoneScale; // 0x7c: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int impactTimingSpriteIndex; // 0x80: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    float impactFeedbackGaugeX; // 0x84: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int powerShotBonusPang; // 0x88: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int pangyaAssistState; // 0x8c: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int preparedUnassistedImpactGaugeBits; // 0x90: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    int unassistedImpactGrade; // 0x94: CRival_ApplyPreparedShot, CRival_PrepareShotGauge
    char pad_098[0x4]; // 0x98
    void* shotProcessor;                       // 0x9c: 2 uses in CRival_Ctor, CRival_dtor
    int savedClubIndex;                         // 0xa0: 1 use in CRival_Ctor
    int selectedClubIndex;                         // 0xa4: 1 use in CRival_Ctor
    float simulationStartFrame; // 0xa8: CRival_SelectClubAndComputeShot
    char difficultyEnabled;                        // 0xac: 1 use in CRival_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CRIVAL_OFF(f, o) typedef char CRival_off_##f[((int)&((CRival*)0)->f == (o)) ? 1 : -1]
CRIVAL_OFF(holePosition, 0x1c);
CRIVAL_OFF(powerShotLevel, 0x28);
CRIVAL_OFF(shotCurve, 0x38);
CRIVAL_OFF(shotSpin, 0x3c);
CRIVAL_OFF(shotPositionThinker, 0x40);
CRIVAL_OFF(strokeUse, 0x44);
CRIVAL_OFF(randomUse, 0x48);
CRIVAL_OFF(unplayableHandler, 0x4c);
CRIVAL_OFF(thinkController, 0x50);
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

#endif  // CLASSES_CRIVAL_H
