// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPADSWINGACCELE_H
#define CLASSES_CPADSWINGACCELE_H

#include "game/types.h"

class CPadSwingAccele {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x180];                // 0x4
    float lastResampledVector[3];       // 0x184: vfunc_23 caches the last input sample; index==0 repeats it
    char pad_190[0x4];                  // 0x190
    float shortHistory[30][3];          // 0x194: vfunc_1 clears 30 vectors; sample update wraps shortHistoryIndex at 30
    int shortHistoryIndex;                        // 0x2fc: 1 use in CPadSwingAccele_ResetSwingState
    float currentAcceleration[3];       // 0x300: vfunc_20 reads current vector; vfunc_21 tests x/y thresholds
    char pad_30C[0x4];                  // 0x30c
    float longHistory[120][3];          // 0x310: vfunc_1 clears 120 vectors; vfunc_20 edits history using longHistoryIndex
    int longHistoryIndex;                        // 0x8b0: 1 use in CPadSwingAccele_ResetSwingState
    float negativeYHoldTime;                      // 0x8b4: 1 use in CPadSwingAccele_ResetSwingState
    float negativeXHoldTime;                      // 0x8b8: 1 use in CPadSwingAccele_ResetSwingState
    float steadyAccelerationTime;                      // 0x8bc: 1 use in CPadSwingAccele_ResetSwingState
    float capturedSamples[512][3];      // 0x8c0: ctor clears 512 vectors; vfunc_11 appends samples during capture
    float correctedSampleX;                     // 0x20c0: 1 use in CPadSwingAccele_SumAbsCorrectedSamplePrefix
    float correctedSampleY;                     // 0x20c4: 1 use in CPadSwingAccele_SumAbsCorrectedSamplePrefix
    float correctedSampleZ;                     // 0x20c8: 1 use in CPadSwingAccele_SumAbsCorrectedSamplePrefix
    char pad_20CC[0x17f4];              // 0x20cc
    float peakAccelerationX0;                     // 0x38c0: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX1;                     // 0x38c4: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX2;                     // 0x38c8: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX3;                     // 0x38cc: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX4;                     // 0x38d0: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX5;                     // 0x38d4: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX6;                     // 0x38d8: 1 use in CPadSwingAccele_ArmCapture
    float peakAccelerationX7;                     // 0x38dc: 1 use in CPadSwingAccele_ArmCapture
    int peakHistoryIndex;                       // 0x38e0: 1 use in CPadSwingAccele_ArmCapture
    int captureZThresholdReached;                       // 0x38e4: 1 use in CPadSwingAccele_ArmCapture
    int captureState;                       // 0x38e8: 1 use in CPadSwingAccele_ArmCapture
    void* captureFrameLimit;                     // 0x38ec: 1 use in CPadSwingAccele_ArmCapture
    int capturedSampleCount;                       // 0x38f0: 1 use in CPadSwingAccele_ArmCapture
    char pad_38F4[0x4];                 // 0x38f4
    int peakSampleIndex;                       // 0x38f8: 1 use in CPadSwingAccele_ArmCapture
    char pad_38FC[0xc];                 // 0x38fc
    int accelerationInRange;                       // 0x3908: 3 uses in CPadSwingAccele_UpdateAccelerationInRangeFlag
    float captureStartAcceleration[3]; // 0x390c: vfunc_11 copies currentAcceleration on first captured sample
    float captureEndAcceleration[3];   // 0x3918: vfunc_11 copies currentAcceleration when capture ends; vfunc_18 returns both
    void* swingVtable;                     // 0x3924: 4 uses in CPadSwingAccele_ResampleNewestFirstBatch

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x184, 0x194, 0x300, 0x310, 0x8c0, 0x20c0, 0x390c, 0x3918
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPADSWINGACCELE_OFF(f, o) typedef char CPadSwingAccele_off_##f[((int)&((CPadSwingAccele*)0)->f == (o)) ? 1 : -1]
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

#endif  // CLASSES_CPADSWINGACCELE_H
