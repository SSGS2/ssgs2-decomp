// CPowerInfo: the shot-distance readout in the HUD.
//
// It owns a font resource, the text buffer it formats, and the shot origin the
// displayed distance is measured from.
#ifndef CLASSES_CPOWERINFO_H
#define CLASSES_CPOWERINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"
#include "game/wvector.h"

class CPowerInfo : public CInfoBase {
public:
    CPowerInfo();

    void DrawShotDistanceYards();

    // Release the font and the text buffer, then optionally delete the object.
    void* Destroy(int deleting);

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }

    WVector shotOrigin; // 0x18
};

#define CPOWERINFO_OFF(f, o) typedef char CPowerInfo_off_##f[((int)&((CPowerInfo*)0)->f == (o)) ? 1 : -1]
CPOWERINFO_OFF(shotOrigin, 0x18);
#undef CPOWERINFO_OFF
typedef char CPowerInfo_size[(sizeof(CPowerInfo) == 0x24) ? 1 : -1];

#endif // CLASSES_CPOWERINFO_H
