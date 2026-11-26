// CGolfBall (class name and member PreData(int) come from the developer export list static_R.sel:
// GolfBall__Fv, PreData__9CGolfBallFi). Wii build: sizeof == 0x198AC (the operator new size in GolfBall()).
// Conservative: only fields the Wii code confirms are named; gaps are padding. Evidence tags:
//   [Reset]  CGolfBall_Reset (golf_ball_predata_and_gallery.cpp)      [ctor] the inlined constructor in GolfBall() (GolfBall__Fv)
//   [PreData] PreData__9CGolfBallFi / CGolfBall_GetOrAllocatePreDataAt  [mgr] other callers (grep GolfBall() users)
//   - no m_moveScaleBase and no m_vectorSlideCount: Wii offsets from 0xA8 on are 4 lower, mass/radius 8 lower.
//   - pre-data records start at 0xC4 (YaPang 0xCC), 900 inline records (YaPang 1500); sExpected record is 0x74
//     bytes in both. The overflow buffer pointer therefore sits at 0x19894 (YaPang 0x2A87C).
//   - total size 0x198AC (YaPang 0x2A894).
#ifndef GAME_GOLFBALL_H
#define GAME_GOLFBALL_H

#include "game/wvector.h"

// One 0x74-byte simulation/playback sample. Reset/PreData establish the size;
// the two launch loops and playback establish the velocity, event and state fields.
// Event 1 carries contactNormal; 3 carries an object-hit sound; 5 starts booster warp.
// Event 2/4 and the subkind domain retain numeric names pending stronger evidence.
struct sExpected {
    WVector m_pos;                         // +00: world XYZ position (+Y up)
    WVector velocity;                      // +0C: world velocity
    float rotation[2];                     // +18: live ball's rotation pair
    char eventSoundName[0x40];             // +20: storage bounded by the event bytes
    unsigned char eventKind, eventSubkind; // +60, +61
    char pad_062[2];
    WVector contactNormal; // +64
    int motionState;       // +70: copy of live m_state
};
typedef char ShotSamplePublicSize[sizeof(sExpected) == 0x74 ? 1 : -1];

class CGolfBall {
public:
    sExpected* PreData(int index); // PreData__9CGolfBallFi

    WVector m_pos;      // 0x00 [Reset] 12-byte copy from the argument
    WVector m_velocity; // 0x0C [Reset] Vec3_Zero(this + 0xC)
    int m_state;        // 0x18 [Reset] = 0
    char unk1C[0x28 - 0x1C];
    int groundTriangleHandle;         // +28: FindGroundTriangleAtPoint; -1 means off mesh
    float m_rotation[2];              // 0x2C [Reset] Vec2_Zero(this + 0x2C)
    float m_initCurve;                // 0x34 [Reset] = constant (all four floats get the same constant)
    float m_initSpin;                 // 0x38 [Reset]
    float m_curve;                    // 0x3C [Reset]
    float m_spin;                     // 0x40 [Reset]
    int m_powerFlags;                 // 0x44 [Reset] = 0
    int m_flags;                      // 0x48 [Reset] = 0
    char unk4C[0x58 - 0x4C];          // not touched by Reset (YaPang: m_spinImpulse WVector)
    unsigned char m_recoveryRequired; // 0x58 [Reset] = 0
    unsigned char m_groundType;       // 0x59 [Reset] = 1
    char unk5A[2];
    int m_lie;          // 0x5C [Reset] = 0
    int m_shotStage;    // 0x60 [Reset] = 0
    int m_preDataCount; // 0x64 [Reset] = 0; [PreData] highest valid record index (clamp), [mgr] ++
    union {
        int m_eventPreDataIndex[16]; // legacy extent includes the timing word
        struct {
            int eventSampleIndices[15];
            float simulationSeconds; // +A4: launch-loop simulation clock
        };
    }; // +68..+A3 [Reset] = -1; +A4 is simulation time, not an event index
    int m_contactMaterialIndex;  // 0xA8 [Reset] = -1
    unsigned char m_bounceCount; // 0xAC [Reset] = 0
    char unkAD;
    short m_firstContactId;  // 0xAE [Reset] = -1
    int m_firstContactIndex; // 0xB0 [Reset] = -1
    short m_lastContactId;   // 0xB4 [Reset] = -1
    char unkB6[2];
    int m_lastContactIndex;   // 0xB8 [Reset] = -1
    float m_mass;             // 0xBC [ctor] = constant (YaPang m_mass)
    float m_radius;           // 0xC0 [ctor] = constant (YaPang m_radius)
    sExpected m_preData[900]; // 0xC4 [PreData] 0x74-byte records; [Reset] memset(this+0xC4, 0, 0x197D0)
    sExpected*
        m_pPreDataOverflow; // 0x19894 [PreData] records >= 900 live here, [ctor] = 0; [Reset] freed (new[] cookie at -0x10)
    unsigned char
        m_overflowBlocks; // 0x19898 [Reset] = 0; [CGolfBall_GetOrAllocatePreDataAt] grows the buffer in blocks of 0x3C records
    char unk19899[3];
    union {
        unsigned char m_holeInCrypt[16];
        struct {
            int holeInEncodedValue, holeInAddressMix, holeInKey;
            char holeInTail[4];
        };
    }; // +1989C: obfuscated hole-in value block
    //   [Reset] writes ~(0 ^ word@0x198A4) at 0x1989C and the 4 bytes at 0x198A0;
    //   [ctor] fills 0x198A4 (u32), 0x198A8 (u16), 0x198AA (u8) with random bytes
    CGolfBall();

    void* NewPreData();
    void ClearBuffer();
    void* GetOrAllocatePreDataAt(int index);

    void Reset(void* arg);
    float GetRadius();
};

// Global accessor: lazily creates the one ball manager. static_R.sel: GolfBall__Fv.
CGolfBall* GolfBall();

#define GOLF_SAMPLE_OFF(f, o) typedef char GolfSample_off_##f[((int)&((sExpected*)0)->f == (o)) ? 1 : -1]
GOLF_SAMPLE_OFF(m_pos, 0x00);
GOLF_SAMPLE_OFF(velocity, 0x0C);
GOLF_SAMPLE_OFF(rotation, 0x18);
GOLF_SAMPLE_OFF(eventSoundName, 0x20);
GOLF_SAMPLE_OFF(eventKind, 0x60);
GOLF_SAMPLE_OFF(eventSubkind, 0x61);
GOLF_SAMPLE_OFF(contactNormal, 0x64);
GOLF_SAMPLE_OFF(motionState, 0x70);
#undef GOLF_SAMPLE_OFF
#define GOLF_BALL_OFF(f, o) typedef char GolfBall_off_##f[((int)&((CGolfBall*)0)->f == (o)) ? 1 : -1]
GOLF_BALL_OFF(groundTriangleHandle, 0x28);
GOLF_BALL_OFF(simulationSeconds, 0xA4);
GOLF_BALL_OFF(m_preData, 0xC4);
GOLF_BALL_OFF(m_pPreDataOverflow, 0x19894);
GOLF_BALL_OFF(holeInEncodedValue, 0x1989C);
GOLF_BALL_OFF(holeInAddressMix, 0x198A0);
GOLF_BALL_OFF(holeInKey, 0x198A4);
#undef GOLF_BALL_OFF
typedef char GolfBall_size_check[sizeof(CGolfBall) == 0x198AC ? 1 : -1];

#endif
