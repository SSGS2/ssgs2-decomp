#ifndef CLASSES_CGUIDELINE_H
#define CLASSES_CGUIDELINE_H

#include "game/types.h"

class CGuideLine {
public:
    CGuideLine();

    static void OnLoad();
    void OnInit();
    void OnProcess(float delta);
    void OnPreLoadInit();
    void OnDestroy();

    static char* GetRTTI();
    void vfunc_1Thunk18();
    void HandleMsg(const int* message);
    void SetEnabled(unsigned char enable);
    static void Draw(void* arg);
    static void AppendGroundSegment(void* arg0, void* segmentStep);

    // One piece of the ground ribbon: the two edge points, the clamped
    // center, and the effect sequence drawn on it.
    struct GuideSegment {
        float left[3];   // 0x00
        float right[3];  // 0x0c
        float center[3]; // 0x18
        void* effect;    // 0x24
    };

    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x14];         // 0x4
    void* secondaryVtbl;        // 0x18: actor interface vtable
    unsigned char wasRequested; // 0x1c: request state from the previous frame
    unsigned char requested;    // 0x1d
    char pad_01E[0x2];          // 0x1e
    int effectCount;            // 0x20
    float opacity;              // 0x24
    unsigned char enabled;      // 0x28
    char pad_029[0x3];          // 0x29
    float remainingTime;        // 0x2c: time left before the line hides itself
    unsigned char simulating;   // 0x30
    char pad_031[0x3];          // 0x31
    GuideSegment segments[80];  // 0x34
    void* bitmap;               // 0xcb4
    char pad_CB8[0x10];         // 0xcb8
    int vertexColor0;           // 0xcc8
    char pad_CCC[0x34];         // 0xccc
    int vertexColor1;           // 0xd00
    char pad_D04[0x34];         // 0xd04
    int vertexColor2;           // 0xd38
    char pad_D3C[0x34];         // 0xd3c
    int vertexColor3;           // 0xd70
    char pad_D74[0x38];         // 0xd74

    ~CGuideLine();
    static void SimulateTrajectory(void* arg);
};

#define CGUIDELINE_OFF(f, o) typedef char CGuideLine_off_##f[((int)&((CGuideLine*)0)->f == (o)) ? 1 : -1]
CGUIDELINE_OFF(secondaryVtbl, 0x18);
CGUIDELINE_OFF(wasRequested, 0x1c);
CGUIDELINE_OFF(effectCount, 0x20);
CGUIDELINE_OFF(opacity, 0x24);
CGUIDELINE_OFF(enabled, 0x28);
CGUIDELINE_OFF(remainingTime, 0x2c);
CGUIDELINE_OFF(simulating, 0x30);
CGUIDELINE_OFF(segments, 0x34);
CGUIDELINE_OFF(bitmap, 0xcb4);
CGUIDELINE_OFF(vertexColor0, 0xcc8);
CGUIDELINE_OFF(vertexColor1, 0xd00);
CGUIDELINE_OFF(vertexColor2, 0xd38);
CGUIDELINE_OFF(vertexColor3, 0xd70);
#undef CGUIDELINE_OFF
typedef char CGuideLine_size[(sizeof(CGuideLine) == 0xDAC) ? 1 : -1];
typedef char CGuideLine_segment_size[(sizeof(CGuideLine::GuideSegment) == 0x28) ? 1 : -1];

#endif // CLASSES_CGUIDELINE_H
