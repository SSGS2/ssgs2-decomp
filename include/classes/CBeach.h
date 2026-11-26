#ifndef CLASSES_CBEACH_H
#define CLASSES_CBEACH_H

#include "game/types.h"

// The water's edge at a hole: a row of wave strips that rise, fall and wait,
// plus the texture they are drawn with.
class CBeach {
public:
    void AppendDefaultStripParameters(unsigned int requested);
    void Draw();

    // Resolve a named wave parameter record, or the course default.
    void LoadWaveParameters(const char* name);
    // Advance every strip's rise/fall/wait phase.
    void UpdateWaves(float deltaSeconds);

    void* vtbl;                 // 0x0: original vtable pointer
    void* stripRows;            // 0x4
    unsigned int stripRowCount; // 0x8
    char pad_00C[0x4];          // 0xc
    unsigned int bitmap;        // 0x10: wave texture
    float waveHeight;           // 0x14: travel amplitude in world units
    float waveDuration;         // 0x18: seconds for one full wave
    float gravity;              // 0x1c: derived coefficient of the integrator
    float riseSeconds;          // 0x20
    float fallSeconds;          // 0x24
    float waitSeconds;          // 0x28
    void* stripParameters;      // 0x2c: strip records, 0x18 bytes each
    unsigned int stripCount;    // 0x30
    char pad_034[0x4];          // 0x34
    void* vertexBuffers;        // 0x38
    char pad_03C[0x14];         // 0x3c
    float maxRowHeightDelta;    // 0x50

    CBeach();

    ~CBeach();
};

#define CBEACH_OFF(f, o) typedef char CBeach_off_##f[((int)&((CBeach*)0)->f == (o)) ? 1 : -1]
CBEACH_OFF(stripRows, 0x4);
CBEACH_OFF(stripRowCount, 0x8);
CBEACH_OFF(bitmap, 0x10);
CBEACH_OFF(waveHeight, 0x14);
CBEACH_OFF(waveDuration, 0x18);
CBEACH_OFF(gravity, 0x1c);
CBEACH_OFF(riseSeconds, 0x20);
CBEACH_OFF(fallSeconds, 0x24);
CBEACH_OFF(waitSeconds, 0x28);
CBEACH_OFF(stripParameters, 0x2c);
CBEACH_OFF(stripCount, 0x30);
CBEACH_OFF(vertexBuffers, 0x38);
CBEACH_OFF(maxRowHeightDelta, 0x50);
#undef CBEACH_OFF

#endif // CLASSES_CBEACH_H
