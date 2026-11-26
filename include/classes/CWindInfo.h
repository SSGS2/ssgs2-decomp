// CWindInfo: the wind readout in the HUD.
//
// It owns a font resource and the text buffer it formats each frame. The class
// is not declared with virtual functions: its vtable comes from the original
// data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CWINDINFO_H
#define CLASSES_CWINDINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

class CWindInfo : public CInfoBase {
public:
    CWindInfo();

    void DrawWindHudText();

    // Release the font and the text buffer, then optionally delete the object.
    void* Destroy(int deleting);

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }
};

#define CWINDINFO_OFF(f, o) typedef char CWindInfo_off_##f[((int)&((CWindInfo*)0)->f == (o)) ? 1 : -1]
#undef CWINDINFO_OFF
typedef char CWindInfo_size[(sizeof(CWindInfo) == 0x18) ? 1 : -1];

#endif // CLASSES_CWINDINFO_H
