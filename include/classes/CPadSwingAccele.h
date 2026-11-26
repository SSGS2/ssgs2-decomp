#ifndef CLASSES_CPADSWINGACCELE_H
#define CLASSES_CPADSWINGACCELE_H

#include "game/types.h"

class CPadSwingAccele {
public:
    float samples[32][3];          // 0x0: newest-first batch of resampled pad samples
    int sampleCount;               // 0x180
    float lastResampledVector[3];  // 0x184: vfunc_23 caches the last input sample; index==0 repeats it
    int mirrorXAxis;               // 0x190: sampled X acceleration is mirrored when nonzero
    float shortHistory[30][3];     // 0x194: vfunc_1 clears 30 vectors; sample update wraps shortHistoryIndex at 30
    int shortHistoryIndex;         // 0x2fc
    float currentAcceleration[3];  // 0x300: vfunc_20 reads current vector; vfunc_21 tests x/y thresholds
    float swingAngle;              // 0x30c: processed swing angle
    float longHistory[120][3];     // 0x310: vfunc_1 clears 120 vectors; vfunc_20 edits history using longHistoryIndex
    int longHistoryIndex;          // 0x8b0
    float negativeYHoldTime;       // 0x8b4
    float negativeXHoldTime;       // 0x8b8
    float steadyAccelerationTime;  // 0x8bc
    float capturedSamples[512][3]; // 0x8c0: ctor clears 512 vectors; vfunc_11 appends samples during capture
    float correctedSampleX;        // 0x20c0
    float correctedSampleY;        // 0x20c4
    float correctedSampleZ;        // 0x20c8
    char pad_20CC[0x17f4];         // 0x20cc
    float peakAccelerationX0;      // 0x38c0
    float peakAccelerationX1;      // 0x38c4
    float peakAccelerationX2;      // 0x38c8
    float peakAccelerationX3;      // 0x38cc
    float peakAccelerationX4;      // 0x38d0
    float peakAccelerationX5;      // 0x38d4
    float peakAccelerationX6;      // 0x38d8
    float peakAccelerationX7;      // 0x38dc
    int peakHistoryIndex;          // 0x38e0
    int captureZThresholdReached;  // 0x38e4
    int captureState;              // 0x38e8
    void* captureFrameLimit;       // 0x38ec
    int capturedSampleCount;       // 0x38f0
    float captureElapsed;          // 0x38f4: seconds since capture started
    int peakSampleIndex;           // 0x38f8
    float swingDirectionAngle;     // 0x38fc
    float peakWidthSamples;        // 0x3900
    float peakMeanMagnitude;       // 0x3904
    int accelerationInRange;       // 0x3908
    float captureStartAcceleration[3]; // 0x390c: vfunc_11 copies currentAcceleration on first captured sample
    float captureEndAcceleration
        [3];           // 0x3918: vfunc_11 copies currentAcceleration when capture ends; vfunc_18 returns both
    void* swingVtable; // 0x3924

    int vfunc_3();

    // Read and clamp the selected pad's samples, mirror X if needed, update both histories, and run swing analysis.
    int ReadAndUpdateSwingHistory(void* arg, float dt);

    // Capture samples until the threshold/time limit completes the swing, then repair and analyze the capture.
    int UpdateCapture(int enabled, float delta);

    // Peak analysis results from the capture analysis.
    float GetPeakMeanMagnitude();
    float GetPeakWidthSamples();
    float vfunc_14();

    // Number of captured swing samples.
    unsigned int GetCapturedSampleCount();

    // Timing-mode-dependent resampler output sample count; takes no object.
    static int GetResampleOutputSampleCount();

    // Copy the stored start and end capture vectors to the two output buffers.
    int CopyCaptureAccelerations(const char* first, const char* second);

    // Test current acceleration against the configured X/Y range and store the result.
    int UpdateAccelerationInRangeFlag();

    // Find the swing peak with five-sample windows, then calculate direction, mean magnitude, and peak width.
    int AnalyzeCapturedSamples(Vec3f* v, int count);

    // Accumulate absolute corrected acceleration per axis; returns at least one as the divisor count.
    int SumAbsoluteCorrectedSamples(float* outX, float* outY, float* outZ, int* outCount);
    // Accumulate absolute corrected acceleration over the requested capture fraction.
    int SumAbsCorrectedSamplePrefix(int* outCount, Vec3f* values, float fraction);

    // Remove held/stationary acceleration bias, clamp axes, and derive the current swing angle.
    int CorrectAccelAndUpdateAngle(int enabled, float delta);

    // Resample one batch of newest-first samples to the timing-mode output count; see the definition for the contract.
    int ResampleNewestFirstBatch(Vec3f* output, Vec3f* values, int inputSampleCount);

    // Handedness: whether sampled X acceleration is mirrored.
    int SetMirrorXAxis(int value);
    int IsMirrorXAxis();
    int ToggleMirrorXAxis();

    // Initialize the object in place, as its owner's constructor does. Returns this.
    CPadSwingAccele* Init();
    int ResetSwingState();

    // Processed swing angle and the acceleration-range flag.
    float GetSwingAngle();
    void* GetAccelerationInRange();

    // Clear the capture buffers, arm capture with its frame limit, and reset peak and endpoint data.
    int ArmCapture(void* frameLimit);

    // Repair clipped acceleration runs on each axis with a cubic Bezier interpolation of surrounding samples.
    // Borrow source/count valid vectors and destination/count writable vectors, with
    // count >= 1. Keep them separate: interpolation re-reads original neighboring
    // samples while replacing earlier destination samples. Returns 1; does not
    // allocate or clamp the interpolated results back to the input clipping range.
    // The object is not used; the vtable passes it as the first argument.
    int RepairClippedAccelerationRuns(Vec3f* destination, Vec3f* source, int count);

    // Optionally delete the object; its sample buffers are embedded in it.
    void* Destroy(int deletingFlag);
};

#define CPADSWINGACCELE_OFF(f, o) typedef char CPadSwingAccele_off_##f[((int)&((CPadSwingAccele*)0)->f == (o)) ? 1 : -1]
CPADSWINGACCELE_OFF(captureElapsed, 0x38f4);
CPADSWINGACCELE_OFF(shortHistoryIndex, 0x2fc);
CPADSWINGACCELE_OFF(lastResampledVector, 0x184);
CPADSWINGACCELE_OFF(shortHistory, 0x194);
CPADSWINGACCELE_OFF(currentAcceleration, 0x300);
CPADSWINGACCELE_OFF(longHistory, 0x310);
CPADSWINGACCELE_OFF(capturedSamples, 0x8c0);
CPADSWINGACCELE_OFF(captureStartAcceleration, 0x390c);
CPADSWINGACCELE_OFF(captureEndAcceleration, 0x3918);
CPADSWINGACCELE_OFF(longHistoryIndex, 0x8b0);
CPADSWINGACCELE_OFF(negativeYHoldTime, 0x8b4);
CPADSWINGACCELE_OFF(negativeXHoldTime, 0x8b8);
CPADSWINGACCELE_OFF(steadyAccelerationTime, 0x8bc);
CPADSWINGACCELE_OFF(correctedSampleX, 0x20c0);
CPADSWINGACCELE_OFF(correctedSampleY, 0x20c4);
CPADSWINGACCELE_OFF(correctedSampleZ, 0x20c8);
CPADSWINGACCELE_OFF(peakAccelerationX0, 0x38c0);
CPADSWINGACCELE_OFF(peakAccelerationX1, 0x38c4);
CPADSWINGACCELE_OFF(peakAccelerationX2, 0x38c8);
CPADSWINGACCELE_OFF(peakAccelerationX3, 0x38cc);
CPADSWINGACCELE_OFF(peakAccelerationX4, 0x38d0);
CPADSWINGACCELE_OFF(peakAccelerationX5, 0x38d4);
CPADSWINGACCELE_OFF(peakAccelerationX6, 0x38d8);
CPADSWINGACCELE_OFF(peakAccelerationX7, 0x38dc);
CPADSWINGACCELE_OFF(peakHistoryIndex, 0x38e0);
CPADSWINGACCELE_OFF(captureZThresholdReached, 0x38e4);
CPADSWINGACCELE_OFF(captureState, 0x38e8);
CPADSWINGACCELE_OFF(captureFrameLimit, 0x38ec);
CPADSWINGACCELE_OFF(capturedSampleCount, 0x38f0);
CPADSWINGACCELE_OFF(peakSampleIndex, 0x38f8);
CPADSWINGACCELE_OFF(accelerationInRange, 0x3908);
CPADSWINGACCELE_OFF(swingVtable, 0x3924);
#undef CPADSWINGACCELE_OFF

#endif // CLASSES_CPADSWINGACCELE_H
