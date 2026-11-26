#ifndef CLASSES_CINGAMESTATUS2D_H
#define CLASSES_CINGAMESTATUS2D_H

#include "game/types.h"

class CIngameStatus2d {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x60]; // 0x4
    float gaugeFill;    // 0x64

    static int LoadLayout(void* arg0, int arg1);
    void SetGaugeEnabled(unsigned int value);
    void SetGaugeFill(float value);
    int ResetElementsAndEnableGauge();
    void SetIconAndLabelElements(int id, int base, int mode, int flag);
    void SetIconLabelGaugePositions(int first, int second, int third);
    int UpdateIconLabelAndGauge();
    int UpdateLayoutAndGauge();

    CIngameStatus2d();
    ~CIngameStatus2d();
};

#define CINGAMESTATUS2D_OFF(f, o) typedef char CIngameStatus2d_off_##f[((int)&((CIngameStatus2d*)0)->f == (o)) ? 1 : -1]
CINGAMESTATUS2D_OFF(gaugeFill, 0x64);
#undef CINGAMESTATUS2D_OFF

#endif // CLASSES_CINGAMESTATUS2D_H
