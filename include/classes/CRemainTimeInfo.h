// CRemainTimeInfo: the remaining-turn-time readout in the HUD.
//
// It owns a font resource and the text buffer it formats. The field names below
// the font follow the other HUD text panels and are inferred from their use.
#ifndef CLASSES_CREMAINTIMEINFO_H
#define CLASSES_CREMAINTIMEINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

class CRemainTimeInfo : public CInfoBase {
public:
    CRemainTimeInfo();

    void ShowRemainingTimeText();

    // Release the font and the text buffer, then optionally delete the object.
    void* Destroy(int deleting);

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }
};

#define CREMAINTIMEINFO_OFF(f, o) typedef char CRemainTimeInfo_off_##f[((int)&((CRemainTimeInfo*)0)->f == (o)) ? 1 : -1]
#undef CREMAINTIMEINFO_OFF
typedef char CRemainTimeInfo_size[(sizeof(CRemainTimeInfo) == 0x18) ? 1 : -1];

#endif // CLASSES_CREMAINTIMEINFO_H
