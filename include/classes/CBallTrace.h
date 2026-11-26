#ifndef CLASSES_CBALLTRACE_H
#define CLASSES_CBALLTRACE_H

#include "game/types.h"

// The common opening of the ball-trail effect records: the original vtable and
// an owned quad renderer.
class CBallTrace {
public:
    ~CBallTrace();

    static void BlowAway();
    static void Generate();

    void* vtbl;          // 0x0: original vtable pointer
    char pad_004[0x110]; // 0x4
    void* quadRenderer;  // 0x114
};

#define CBALLTRACE_OFF(f, o) typedef char CBallTrace_off_##f[((int)&((CBallTrace*)0)->f == (o)) ? 1 : -1]
CBALLTRACE_OFF(quadRenderer, 0x114);
#undef CBALLTRACE_OFF
typedef char CBallTrace_size[(sizeof(CBallTrace) == 0x118) ? 1 : -1];

#endif // CLASSES_CBALLTRACE_H
