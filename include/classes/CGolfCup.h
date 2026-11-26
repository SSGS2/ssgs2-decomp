// CGolfCup: the cup that is shown before a round.
//
// It owns the cup model, the 128x128 capture queue and the texture captured
// into it, the ground shadow mesh, the capture camera and its projection, and
// the material references kept alive while a capture runs.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CGOLFCUP_H
#define CLASSES_CGOLFCUP_H

#include <decomp/cw_bitfields.h>
#include "game/types.h"

// A resource block with a 16-bit reference count. The capture texture and the
// model's materials share this representation.
struct Ref {
    CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
};

struct RefPtr {
    Ref* ptr;
};

class CGolfCup {
public:
    CGolfCup();
    ~CGolfCup();

    void* vtbl; // 0x00
    char pad_004[0x0C];
    void* model;          // 0x10  model puppet owned by the resource manager
    void* captureQueue;   // 0x14  queue the preview is drawn into
    unsigned char active; // 0x18  the captured texture is current
    char pad_019[3];
    Vec3f pos;             // 0x1C  world-space position
    void* shadowMesh;      // 0x28
    RefPtr captureTexture; // 0x2C  first texture of the capture queue
    float camera[16];      // 0x30  world-to-capture-view matrix
    float projection[16];  // 0x70  capture projection matrix
    void* buffer;          // 0xB0  material references kept during a capture
    unsigned int size;     // 0xB4  entries in buffer
    unsigned int capacity; // 0xB8  entries allocated
};

#define CGOLFCUP_OFFSET(field, offset) \
    typedef char CGolfCup_offset_##field[((int)&((CGolfCup*)0)->field == (offset)) ? 1 : -1]
CGOLFCUP_OFFSET(model, 0x10);
CGOLFCUP_OFFSET(active, 0x18);
CGOLFCUP_OFFSET(pos, 0x1C);
CGOLFCUP_OFFSET(captureTexture, 0x2C);
CGOLFCUP_OFFSET(camera, 0x30);
CGOLFCUP_OFFSET(projection, 0x70);
CGOLFCUP_OFFSET(buffer, 0xB0);
CGOLFCUP_OFFSET(capacity, 0xB8);
#undef CGOLFCUP_OFFSET
typedef char CGolfCup_size[(sizeof(CGolfCup) == 0xBC) ? 1 : -1];

#endif // CLASSES_CGOLFCUP_H
