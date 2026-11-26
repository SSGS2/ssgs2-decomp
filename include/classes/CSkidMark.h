// CSkidMark: the terrain-hugging marks a shot leaves behind.
//
// The mark is a ribbon of projected rows, one per ball-history sample in the
// requested frame range, each with its own opacity. The bitmap and the quad
// renderer are owned by the object.
#ifndef CLASSES_CSKIDMARK_H
#define CLASSES_CSKIDMARK_H

#include "game/types.h"

class CSkidMark {
public:
    ~CSkidMark();

    void AllocateFrameRange();
    void UpdateFromBallHistory();
    void Draw();

    void* vtbl;             // 0x00
    char pad_004[0x4];      // 0x04
    void* textureResource;  // 0x08
    char pad_00C[0x10];     // 0x0C
    unsigned char enabled;  // 0x1C
    char pad_01D[0xF7];     // 0x1D
    void* quadRenderer;     // 0x114
    void* trailPoints;      // 0x118  rows of two world-space edges and an alpha byte
    int pointCapacity;      // 0x11C  allocated rows
    int pointCount;         // 0x120  populated rows
    int startFrame;         // 0x124  first history sample
    int endFrame;           // 0x128  last history sample
    int lastProcessedFrame; // 0x12C
};

#define CSKIDMARK_OFF(f, o) typedef char CSkidMark_off_##f[((int)&((CSkidMark*)0)->f == (o)) ? 1 : -1]
CSKIDMARK_OFF(textureResource, 0x8);
CSKIDMARK_OFF(enabled, 0x1C);
CSKIDMARK_OFF(quadRenderer, 0x114);
CSKIDMARK_OFF(trailPoints, 0x118);
CSKIDMARK_OFF(pointCapacity, 0x11C);
CSKIDMARK_OFF(pointCount, 0x120);
CSKIDMARK_OFF(startFrame, 0x124);
CSKIDMARK_OFF(endFrame, 0x128);
CSKIDMARK_OFF(lastProcessedFrame, 0x12C);
#undef CSKIDMARK_OFF

#endif // CLASSES_CSKIDMARK_H
