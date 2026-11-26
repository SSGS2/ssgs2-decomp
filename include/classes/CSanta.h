#ifndef CLASSES_CSANTA_H
#define CLASSES_CSANTA_H

#include "game/types.h"

// CSanta: the flying Santa course creature. It follows an authored waypoint
// list with a random offset at each waypoint, can record and replay its path,
// banks into turns, and plays a boost sound that fades with camera distance.
//
// The flight update uses this layout; the other member functions still reach
// the object's data through a file-local layout. The vtable comes from the
// original game data, so it is spelled out as a member.
class CSanta {
public:
    // Advance Santa animation events, occasionally switch its default clip, then update movement.
    void UpdateAnimationAndMovement(float dt);

    // Follow or record waypoints, bank into turns, and scale the boost sound by camera distance.
    void UpdateFlightMovement(float dt);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    // Initialize state and owned resources (the original constructor; not a C++ constructor).
    void* Init(const char* arg);

    // Release resources and optionally delete the object.
    void* Dtor(int del);

    void DtorThunk4(int deleteFlag);

    void* vtbl;                    // 0x00
    void* secondaryVtbl;           // 0x04
    unsigned int flags;            // 0x08
    float movementBoundsOrigin[3]; // 0x0C
    float movementBoundsMax[3];    // 0x18
    void* puppet;                  // 0x24
    float basis[6];                // 0x28  first two rows of the transform
    float forward[3];              // 0x40  third row; Santa flies along its negative
    float position[3];             // 0x4C  translation row
    float animationFrame;          // 0x58
    char pad_05C[4];
    void* animationRange; // 0x60
    float target[3];      // 0x64
    float turnRate;       // 0x70  radians per time unit
    float movementSpeed;  // 0x74
    float bankAngle;      // 0x78
    float maxBankAngle;   // 0x7C
    float bankRate;       // 0x80
    float heading;        // 0x84
    char pad_088[0x20];
    void* history; // 0xA8
    char pad_0AC;
    unsigned char animationActive; // 0xAD
    char pad_0AE[2];
    float ballSpawnOffsetDistance; // 0xB0  radius of the random offset added to each waypoint
    char pad_0B4[0xC];
    void* targetPoints;               // 0xC0  authored waypoints, 12 bytes each
    unsigned int targetPointCount;    // 0xC4
    unsigned int targetPointCapacity; // 0xC8
    unsigned char targetCursor;       // 0xCC
    char pad_0CD[3];
    void* pathPoints;               // 0xD0  recorded path, 12 bytes each
    unsigned int pathPointCount;    // 0xD4
    unsigned int pathPointCapacity; // 0xD8
    unsigned char pathCursor;       // 0xDC  replay position in the recorded path
    char pad_0DD[0x3C];
    unsigned char recordPath; // 0x119
    char pad_11A[2];
    float verticalSpeed; // 0x11C
    int bankPhase;       // 0x120
};

#define CSANTA_OFF(f, o) typedef char CSanta_off_##f[((int)&((CSanta*)0)->f == (o)) ? 1 : -1]
CSANTA_OFF(puppet, 0x24);
CSANTA_OFF(position, 0x4C);
CSANTA_OFF(target, 0x64);
CSANTA_OFF(ballSpawnOffsetDistance, 0xB0);
CSANTA_OFF(pathCursor, 0xDC);
CSANTA_OFF(recordPath, 0x119);
CSANTA_OFF(bankPhase, 0x120);
#undef CSANTA_OFF

#endif // CLASSES_CSANTA_H
