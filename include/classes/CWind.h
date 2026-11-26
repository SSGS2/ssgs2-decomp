// CWind: the wind on the course.
//
// One instance exists for the round. It holds the global wind as a speed and a
// direction, the local wind regions loaded from the course, the "Silvia Cannon"
// that replaces the wind on the holes that have one, and the wind the HUD arrow
// shows.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CWIND_H
#define CLASSES_CWIND_H

#include "game/types.h"
#include "game/wvector.h"

// A box on the course with its own wind. Bounds are inclusive world-space
// corners; the wind is stored as a vector and rescaled whenever the global
// speed changes. The record is copied by value inside the region list.
struct WindRegion {
    // How the region's strength combines with the global speed. The other two
    // values are applied only when the region is parsed.
    enum StrengthMode {
        kAddToGlobalSpeed = 0,
        kScaleGlobalSpeed = 1,
        kSubtractFromGlobalSpeed = 2,
        kReplaceGlobalSpeed = 3
    };

    int index;                  // 0x00  position in the region list
    WVector minimum;            // 0x04
    WVector maximum;            // 0x10
    WVector wind;               // 0x1C
    unsigned char strengthMode; // 0x28
    char pad_29[3];
    float strength;     // 0x2C
    float yawDegrees;   // 0x30
    float pitchDegrees; // 0x34
    int adjacentRegion; // 0x38  index of the one region that overlaps this one, or -1
};

class CWind {
public:
    CWind();
    ~CWind();

    // The shared instance, built on first use.
    static CWind* GetInstance();

    // Speed and direction.
    void SetWindParams(unsigned char strengthByte, unsigned char directionByte, int unused, int resetWind);
    void SetWindVector(const WVector& wind);
    void SetStrength(float speed);
    WVector GetWindVector();
    float GetWindSpeed() const;
    float GetDirectionAngle();

    // Local wind regions.
    void LoadCourseRegions();
    void RescaleRegions();
    void AddRegionFromCommand(void* bounds, const char* command);
    bool ContainsPoint(const WVector& point);
    WVector GetWindVectorAtPoint(const WVector& point, float maxBaseMagnitude);

    // Silvia Cannon.
    void InitOverrideFromCourse();
    void SetOverrideEnabled(unsigned int enable);
    void SetEffectEnabled(int enable);
    void PlayCannonEndSoundOnce();
    void PlayCannonChargeSoundOnce();

    // HUD wind arrow.
    void UpdateIndicator(float deltaSeconds);
    void DrawDirectionModel(void* arrowModel);

    // What the arrow's model is given when it is set up, with the arrow's
    // direction in the middle of it. The model's setup hook ignores the rest.
    struct ModelSetup {
        unsigned int word;                // 0x64
        WVector direction;                // 0x68
        unsigned int wordA, wordB, wordC; // 0x74
        float indicatorDirection;         // 0x80  arrow direction relative to the view
        float scalarA, scalarB;           // 0x84
    };

    // The arrow's own viewport: pixel rectangle and depth range.
    struct IndicatorViewport {
        unsigned int x, y, width, height; // 0x100
        float minimumDepth, maximumDepth; // 0x110
    };

    // Light 7, which lights the arrow.
    struct IndicatorLight {
        unsigned int type; // 0x118
        float diffuse[4];  // 0x11C
        float specular[4]; // 0x12C
        float ambient[4];  // 0x13C
        char pad_34[0xC];
        WVector direction; // 0x158
        char pad_4C[0x1C];
    };

    void* vtbl;                            // 0x00
    WVector additiveWindTarget;            // 0x04  additiveWind eases toward this
    WVector additiveWind;                  // 0x10  added to the base wind; party items set it
    WVector indicatorWind;                 // 0x1C  wind at the ball, or at the camera before the shot
    float speed;                           // 0x28
    float directionAngle;                  // 0x2C  radians about the Y axis
    float indicatorPhase;                  // 0x30  animation phase, advances with wind speed
    float indicatorViewRotation[9];        // 0x34  with indicatorViewPosition: the arrow camera's 3x4 transform
    WVector indicatorViewPosition;         // 0x58
    ModelSetup modelSetup;                 // 0x64
    unsigned char enabled;                 // 0x8C
    unsigned char cannonEndSoundPlayed;    // 0x8D
    unsigned char cannonChargeSoundPlayed; // 0x8E
    char pad_08F;
    WindRegion* regions;          // 0x90
    int regionCount;              // 0x94
    unsigned int regionCapacity;  // 0x98
    WVector windVector;           // 0x9C  speed and direction as a vector
    WVector overrideVector;       // 0xA8  the cannon's wind
    float overrideSpeed;          // 0xB4  negative when the hole has no cannon
    float overrideDirection;      // 0xB8
    unsigned char overrideActive; // 0xBC  the cannon is firing
    char pad_0BD[3];
    float effectRotation[9];             // 0xC0  with effectPosition: the cannon effect's 3x4 transform
    WVector effectPosition;              // 0xE4
    WVector effectOffset;                // 0xF0
    void* attachedEffect;                // 0xFC
    IndicatorViewport indicatorViewport; // 0x100
    IndicatorLight indicatorLight;       // 0x118
};

#define CWIND_OFFSET(field, offset) typedef char CWind_offset_##field[((int)&((CWind*)0)->field == (offset)) ? 1 : -1]
CWIND_OFFSET(additiveWind, 0x10);
CWIND_OFFSET(speed, 0x28);
CWIND_OFFSET(indicatorViewRotation, 0x34);
CWIND_OFFSET(indicatorViewPosition, 0x58);
CWIND_OFFSET(modelSetup, 0x64);
CWIND_OFFSET(enabled, 0x8C);
CWIND_OFFSET(regions, 0x90);
CWIND_OFFSET(windVector, 0x9C);
CWIND_OFFSET(overrideSpeed, 0xB4);
CWIND_OFFSET(overrideActive, 0xBC);
CWIND_OFFSET(effectPosition, 0xE4);
CWIND_OFFSET(attachedEffect, 0xFC);
CWIND_OFFSET(indicatorViewport, 0x100);
CWIND_OFFSET(indicatorLight, 0x118);
#undef CWIND_OFFSET
typedef char CWind_size[(sizeof(CWind) == 0x180) ? 1 : -1];
typedef char CWind_modelSetup_size[(sizeof(CWind::ModelSetup) == 0x28) ? 1 : -1];
typedef char CWind_light_size[(sizeof(CWind::IndicatorLight) == 0x68) ? 1 : -1];
typedef char WindRegion_size[(sizeof(WindRegion) == 0x3C) ? 1 : -1];

#endif // CLASSES_CWIND_H
