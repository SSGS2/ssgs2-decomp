// CPuttLine: the ribbon drawn along the putting line.
//
// One segment per ball-history sample; the segment's color word is either an
// opacity or a selector for the special putting surface color.
#ifndef CLASSES_CPUTTLINE_H
#define CLASSES_CPUTTLINE_H

#include "game/types.h"
#include "game/wvector.h"

struct PuttRibbonSegment {
    int colorOrOpacity;
    WVector left;
    WVector right;
};

class CPuttLine {
public:
    ~CPuttLine();

    void Init();
    void Process();
    void Display();

    void* vtbl;                  // 0x00
    char pad_004[0x4];           // 0x04
    void* textureId;             // 0x08
    char pad_00C[0x10];          // 0x0C
    unsigned char active;        // 0x1C
    char pad_01D[0xF7];          // 0x1D
    void* polygon;               // 0x114
    PuttRibbonSegment* segments; // 0x118
};

#define CPUTTLINE_OFF(f, o) typedef char CPuttLine_off_##f[((int)&((CPuttLine*)0)->f == (o)) ? 1 : -1]
CPUTTLINE_OFF(textureId, 0x8);
CPUTTLINE_OFF(active, 0x1C);
CPUTTLINE_OFF(polygon, 0x114);
CPUTTLINE_OFF(segments, 0x118);
#undef CPUTTLINE_OFF
typedef char PuttRibbonSegment_size[(sizeof(PuttRibbonSegment) == 0x1C) ? 1 : -1];

#endif // CLASSES_CPUTTLINE_H
