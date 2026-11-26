// CShotCountInfo: the shot counter in the HUD.
//
// It owns a font resource and the text buffer it formats, and fades the counter
// out in fixed alpha steps. The class is not declared with virtual functions:
// its vtable comes from the original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CSHOTCOUNTINFO_H
#define CLASSES_CSHOTCOUNTINFO_H

#include "game/types.h"
#include "classes/HudTextPanel.h"
#include "classes/CInfoBase.h"

class CShotCountInfo : public CInfoBase {
public:
    CShotCountInfo();
    ~CShotCountInfo();

    void ResetFade();
    void UpdateFade(float delta);

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }

    unsigned char alpha; // 0x18
    char pad_019[3];     // 0x19
    float timer;         // 0x1C
    int counter;         // 0x20
};

#define CSHOTCOUNTINFO_OFF(f, o) typedef char CShotCountInfo_off_##f[((int)&((CShotCountInfo*)0)->f == (o)) ? 1 : -1]
CSHOTCOUNTINFO_OFF(alpha, 0x18);
CSHOTCOUNTINFO_OFF(timer, 0x1C);
CSHOTCOUNTINFO_OFF(counter, 0x20);
#undef CSHOTCOUNTINFO_OFF
typedef char CShotCountInfo_size[(sizeof(CShotCountInfo) == 0x24) ? 1 : -1];

#endif // CLASSES_CSHOTCOUNTINFO_H
