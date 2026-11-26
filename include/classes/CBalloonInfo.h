// CBalloonInfo: the balloon-mode pang popup in the HUD.
//
// It owns a font resource, a text buffer, the popup overlay and the texture
// rectangle it draws it with. The popup counts a pending pang reward into the
// current player's total over a short animation. The names below the HUD text
// field are inferred from the popup's construction and animation. The class is
// declared without virtual functions on purpose: its vtable is still taken from
// the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CBALLOONINFO_H
#define CLASSES_CBALLOONINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"

class CBalloonInfo : public CInfoBase {
public:
    CBalloonInfo();
    ~CBalloonInfo();

    // Animate the pending pang into the player's total and commit it.
    void AnimateAndCommitPang(float delta);

    // Reset the popup position, opacity, pending pang and animation timers.
    void ResetPopupAnimation();

    // Draw the popup with its animated scale.
    void DrawPangPopup();

    // Callback slots for the balloon panel; both do nothing else.
    void vfunc_6();
    int vfunc_7();

    int pendingPang;         // 0x18
    int textOpacity;         // 0x1C
    int animationState;      // 0x20
    float popupX;            // 0x24
    float popupY;            // 0x28
    float phaseElapsed;      // 0x2C
    float countElapsed;      // 0x30
    int startingTotal;       // 0x34
    void* popupOverlay;      // 0x38
    volatile float uvX;      // 0x3C
    volatile float uvY;      // 0x40
    volatile float uvWidth;  // 0x44
    volatile float uvHeight; // 0x48
};

#define CBALLOONINFO_OFF(f, o) typedef char CBalloonInfo_off_##f[((int)&((CBalloonInfo*)0)->f == (o)) ? 1 : -1]
CBALLOONINFO_OFF(pendingPang, 0x18);
CBALLOONINFO_OFF(textOpacity, 0x1C);
CBALLOONINFO_OFF(animationState, 0x20);
CBALLOONINFO_OFF(popupX, 0x24);
CBALLOONINFO_OFF(popupY, 0x28);
CBALLOONINFO_OFF(phaseElapsed, 0x2C);
CBALLOONINFO_OFF(countElapsed, 0x30);
CBALLOONINFO_OFF(startingTotal, 0x34);
CBALLOONINFO_OFF(popupOverlay, 0x38);
CBALLOONINFO_OFF(uvX, 0x3C);
CBALLOONINFO_OFF(uvY, 0x40);
CBALLOONINFO_OFF(uvWidth, 0x44);
CBALLOONINFO_OFF(uvHeight, 0x48);
#undef CBALLOONINFO_OFF
typedef char CBalloonInfo_size[(sizeof(CBalloonInfo) == 0x4C) ? 1 : -1];

#endif // CLASSES_CBALLOONINFO_H
