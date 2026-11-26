// CComUnplayable: the controller for the unplayable-ball drop.
//
// It holds the drop sequence state, the chosen drop position, the thinker that
// picks it, and the rival that resumes once the ball is placed. The vtable
// pointer is part of the object in the original, so it is spelled out as a
// plain member rather than declared virtual.
#ifndef CLASSES_CCOMUNPLAYABLE_H
#define CLASSES_CCOMUNPLAYABLE_H

#include "game/types.h"

class CComUnplayable {
public:
    int IsDropSequenceActive();
    int UpdateDropSequence();
    void ResetToIdle();
    static int BeginDropSequence(CComUnplayable* __restrict self);

    CComUnplayable(int rival, int dropThinker);
    ~CComUnplayable();

    void* vtbl;               // 0x0: original vtable pointer
    int state;                // 0x4
    float dropPosition[3];    // 0x8: the chosen drop point
    unsigned int startTimeMs; // 0x14: when the drop camera was requested
    int shotPositionThinker;  // 0x18
    int rival;                // 0x1c
};

#define CCOMUNPLAYABLE_OFF(f, o) typedef char CComUnplayable_off_##f[((int)&((CComUnplayable*)0)->f == (o)) ? 1 : -1]
CCOMUNPLAYABLE_OFF(state, 0x4);
CCOMUNPLAYABLE_OFF(dropPosition, 0x8);
CCOMUNPLAYABLE_OFF(startTimeMs, 0x14);
CCOMUNPLAYABLE_OFF(shotPositionThinker, 0x18);
CCOMUNPLAYABLE_OFF(rival, 0x1c);
#undef CCOMUNPLAYABLE_OFF
typedef char CComUnplayable_size[(sizeof(CComUnplayable) == 0x20) ? 1 : -1];

#endif // CLASSES_CCOMUNPLAYABLE_H
