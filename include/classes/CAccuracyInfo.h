// CAccuracyInfo: the impact-message panel in the HUD.
//
// It owns a message overlay, two text buffers and the panel that draws the
// impact labels. The class is not declared with virtual functions: its vtable
// comes from the original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CACCURACYINFO_H
#define CLASSES_CACCURACYINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

class CAccuracyInfo : public CInfoBase {
public:
    CAccuracyInfo();
    ~CAccuracyInfo();

    void FadeOutMessages(float amount);
    void ShowFullPower();
    void ShowCurrentShotFeedback();
    void DrawMessages();
    void ShowShotFlagFeedback(int flags, int mode);

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }

    unsigned char active;   // 0x18
    char pad_019[3];        // 0x19
    float opacity;          // 0x1C
    void* textOverlay;      // 0x20
    float gaugeOffsetX;     // 0x24
    char* secondaryMessage; // 0x28
};

#define CACCURACYINFO_OFF(f, o) typedef char CAccuracyInfo_off_##f[((int)&((CAccuracyInfo*)0)->f == (o)) ? 1 : -1]
CACCURACYINFO_OFF(active, 0x18);
CACCURACYINFO_OFF(opacity, 0x1C);
CACCURACYINFO_OFF(textOverlay, 0x20);
CACCURACYINFO_OFF(gaugeOffsetX, 0x24);
CACCURACYINFO_OFF(secondaryMessage, 0x28);
#undef CACCURACYINFO_OFF
typedef char CAccuracyInfo_size[(sizeof(CAccuracyInfo) == 0x2C) ? 1 : -1];

#endif // CLASSES_CACCURACYINFO_H
