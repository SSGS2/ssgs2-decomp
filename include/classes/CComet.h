// CComet: the tapered trail behind a shot.
//
// The trail is a ring of world-space edge pairs; the newest edge is committed
// on a sample timer, and the live head edge between updates is stored apart
// from the ring. The bitmap and the quad renderer are owned by the object.
#ifndef CLASSES_CCOMET_H
#define CLASSES_CCOMET_H

#include "game/types.h"
#include "game/wvector.h"

// One trail cross-section: the two ends of the ribbon at one sample.
struct BallRibbonEdge {
    WVector left;
    WVector right;
};

class CComet {
public:
    CComet();
    ~CComet();

    void StartTrail();
    void CommitTrailEdge();
    void UpdateTrail(float delta);
    void DrawTrail();

    void* vtbl;               // 0x000
    int clearedWord04;        // 0x004
    int clearedWord08;        // 0x008
    int trailCapacity;        // 0x00C
    int trailIndex;           // 0x010  newest committed edge
    float sampleElapsed;      // 0x014
    float sampleInterval;     // 0x018
    unsigned char active;     // 0x01C
    char pad_01D[0xF7];       // 0x01D
    void* material;           // 0x114
    BallRibbonEdge edges[20]; // 0x118  ring of committed edges
    float trailLength;        // 0x2F8  committed edges, truncated for drawing
    char pad_2FC[0x4];        // 0x2FC
    float brightness;         // 0x300
    void* bitmap;             // 0x304
    int hasPendingTrailEdge;  // 0x308
    WVector headLeft;         // 0x30C  live head edge, before it is committed
    WVector headRight;        // 0x318
};

#define CCOMET_OFF(f, o) typedef char CComet_off_##f[((int)&((CComet*)0)->f == (o)) ? 1 : -1]
CCOMET_OFF(trailCapacity, 0xC);
CCOMET_OFF(trailIndex, 0x10);
CCOMET_OFF(sampleElapsed, 0x14);
CCOMET_OFF(sampleInterval, 0x18);
CCOMET_OFF(active, 0x1C);
CCOMET_OFF(material, 0x114);
CCOMET_OFF(edges, 0x118);
CCOMET_OFF(trailLength, 0x2F8);
CCOMET_OFF(brightness, 0x300);
CCOMET_OFF(bitmap, 0x304);
CCOMET_OFF(hasPendingTrailEdge, 0x308);
CCOMET_OFF(headLeft, 0x30C);
CCOMET_OFF(headRight, 0x318);
#undef CCOMET_OFF
typedef char CComet_size[(sizeof(CComet) == 0x324) ? 1 : -1];
typedef char BallRibbonEdge_size[(sizeof(BallRibbonEdge) == 0x18) ? 1 : -1];

#endif // CLASSES_CCOMET_H
