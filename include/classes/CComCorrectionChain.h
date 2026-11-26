// The correction helpers owned by the rival's shot pipeline.
//
// A CComPadOnePoint or CComShotOnePoint task holds one angle correction and one
// power correction child, linked as one correction chain. A correction steps one
// parameter of a shot candidate across a fixed range, simulates each sample and
// keeps the better result. The processor installs each concrete vtable before
// assigning the fields, and destroys the chains when it is destroyed.
#ifndef CLASSES_CCOMCORRECTIONCHAIN_H
#define CLASSES_CCOMCORRECTIONCHAIN_H

#include <decomp/local_class.h>
#include "classes/CComShotProc.h"

class CRival;

// The range a correction sweeps: half-width and number of samples. The power
// correction's range record carries two words before the half-width.
struct AngleSampleRange {
    float halfRange; // 0x00
    int sampleCount; // 0x04
};
struct PowerSampleRange {
    char pad_00[8];
    float halfRange; // 0x08
    int sampleCount; // 0x0C
};

// Children are destroyed through the deleting-destructor slot of their vtable.
struct CorrectionChild {
    virtual void DestroyNow(int deleting) DECOMP_LOCAL_VIRTUAL;
};

// Angle correction: steps the aim angle by a fixed increment per sample.
class CComShotAngleCorrection {
public:
    ~CComShotAngleCorrection();

    // Advance the sample index and report whether more samples remain.
    unsigned NextAngleSample();
    // Log the angle-corrected candidate and its landing status.
    void LogCandidate();
    // Copy the current candidate and establish the angle sampling interval.
    void InitSampleFromCandidate();
    // Simulate one angle sample and retain the preferred candidate.
    void EvaluateAngleSample();
    // Vtable slot 5: set the flag that decides whether a sample that hits is
    // still logged.
    void SetEnabled(int value);

    void* vtable;                 // 0x00
    void* next;                   // 0x04
    void* child;                  // 0x08
    AngleSampleRange* config;     // 0x0C
    CRival* owner;                // 0x10
    int sampleIndex;              // 0x14
    ShotCandidate* bestCandidate; // 0x18
    ShotCandidate sample;         // 0x1C  the candidate payload being sampled
    float minimumAngle;           // 0x5C
    int enabled;                  // 0x60
};
typedef char CComShotAngleCorrection_size[(sizeof(CComShotAngleCorrection) == 0x64) ? 1 : -1];

// Power correction: steps the shot power by a fixed increment per sample.
class CComShotPowerCorrection {
public:
    ~CComShotPowerCorrection();

    // Log the candidate the power sweep settled on.
    void LogCandidate();
    // Advance the sample index and report whether more samples remain.
    int NextPowerSample();
    // Clamp and simulate one power sample and retain the preferred candidate.
    void SimulatePowerSample();
    // Copy the current candidate and establish the power sampling interval.
    void InitSampleState();
    // Vtable slot 5: set the flag that decides whether a sample that hits is
    // still logged.
    void SetEnabled(int value);

    void* vtable;                 // 0x00
    void* next;                   // 0x04
    void* child;                  // 0x08
    PowerSampleRange* config;     // 0x0C
    CRival* owner;                // 0x10
    int sampleIndex;              // 0x14
    ShotCandidate* bestCandidate; // 0x18
    ShotCandidate sample;         // 0x1C  the candidate payload being sampled
    float minimumPower;           // 0x5C
    float maximumPower;           // 0x60
    int enabled;                  // 0x64
};
typedef char CComShotPowerCorrection_size[(sizeof(CComShotPowerCorrection) == 0x68) ? 1 : -1];

// One-point correction task for putts. It reads the processor state word.
class CComPadOnePoint {
public:
    ~CComPadOnePoint();

    // Alternate the correction axis and report whether more directions remain.
    unsigned AdvanceCorrectionDirection();
    // Configure the current putt correction child and activate its task chain.
    void ActivateCorrectionTask();
    // Apply character-dependent putt error and update the rival aim and cached shot gauge.
    void ApplyPuttResultToRival();
    // Estimate putt heading and power from target distance and sampled terrain slope.
    void InitializePuttCandidate();

    void* vtable;                             // 0x00
    void* next;                               // 0x04
    void* child;                              // 0x08
    int correctionAxis;                       // 0x0C
    void* processorState;                     // 0x10  the processor's +0x0C state
    int directionIndex;                       // 0x14
    CRival* owner;                            // 0x18
    CComShotAngleCorrection* angleCorrection; // 0x1C
    CComShotPowerCorrection* powerCorrection; // 0x20
};
typedef char CComPadOnePoint_size[(sizeof(CComPadOnePoint) == 0x24) ? 1 : -1];

// One-point correction task for full shots.
class CComShotOnePoint {
public:
    ~CComShotOnePoint();

    // Alternate the correction axis and report whether the sweep should continue.
    int NextCorrectionDirection();
    // Aim at the current target, choose a club and start a fresh candidate.
    void SelectClubForTargetHeading();
    // Configure and activate the angle or power correction child for this direction.
    void ConfigureAndActivateCorrection();
    // Validate landing constraints, update the fallback candidate, or accept the simulated shot.
    void EvaluateShotResult(void* result);

    void* vtable;                             // 0x00
    void* next;                               // 0x04
    void* child;                              // 0x08
    void* candidate;                          // 0x0C  candidate the chain is tuning
    unsigned directionIndex;                  // 0x10
    int correctionAxis;                       // 0x14
    CRival* owner;                            // 0x18
    CComShotAngleCorrection* angleCorrection; // 0x1C
    CComShotPowerCorrection* powerCorrection; // 0x20
};
typedef char CComShotOnePoint_size[(sizeof(CComShotOnePoint) == 0x24) ? 1 : -1];

#define CCOMCORRECTION_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
CCOMCORRECTION_OFF(CComShotAngleCorrection, config, 0x0C);
CCOMCORRECTION_OFF(CComShotAngleCorrection, sampleIndex, 0x14);
CCOMCORRECTION_OFF(CComShotAngleCorrection, bestCandidate, 0x18);
CCOMCORRECTION_OFF(CComShotAngleCorrection, sample, 0x1C);
CCOMCORRECTION_OFF(CComShotAngleCorrection, minimumAngle, 0x5C);
CCOMCORRECTION_OFF(CComShotAngleCorrection, enabled, 0x60);
CCOMCORRECTION_OFF(CComShotPowerCorrection, config, 0x0C);
CCOMCORRECTION_OFF(CComShotPowerCorrection, sampleIndex, 0x14);
CCOMCORRECTION_OFF(CComShotPowerCorrection, bestCandidate, 0x18);
CCOMCORRECTION_OFF(CComShotPowerCorrection, sample, 0x1C);
CCOMCORRECTION_OFF(CComShotPowerCorrection, minimumPower, 0x5C);
CCOMCORRECTION_OFF(CComShotPowerCorrection, maximumPower, 0x60);
CCOMCORRECTION_OFF(CComShotPowerCorrection, enabled, 0x64);
CCOMCORRECTION_OFF(CComPadOnePoint, correctionAxis, 0x0C);
CCOMCORRECTION_OFF(CComPadOnePoint, processorState, 0x10);
CCOMCORRECTION_OFF(CComPadOnePoint, directionIndex, 0x14);
CCOMCORRECTION_OFF(CComShotOnePoint, candidate, 0x0C);
CCOMCORRECTION_OFF(CComShotOnePoint, directionIndex, 0x10);
CCOMCORRECTION_OFF(CComShotOnePoint, correctionAxis, 0x14);
#undef CCOMCORRECTION_OFF

#endif // CLASSES_CCOMCORRECTIONCHAIN_H
