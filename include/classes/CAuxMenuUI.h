// CAuxMenuUI: the auxiliary menu. It holds three overlays, loaded by its
// constructor and deleted by Destroy.
//
// The class has no virtual functions: its vtable comes from the original game
// data, so vtbl is spelled out as the first member.
#ifndef CLASSES_CAUXMENUUI_H
#define CLASSES_CAUXMENUUI_H

class CAuxMenuUI {
public:
    CAuxMenuUI();

    // Delete the three overlays, then the object itself when shouldDelete is positive.
    void* Destroy(int shouldDelete);

    void* vtbl;          // 0x0: __vt__10CAuxMenuUI
    void* firstOverlay;  // 0x4: first WOverlay_Create result
    void* secondOverlay; // 0x8: second WOverlay_Create result
    void* thirdOverlay;  // 0xC: third WOverlay_Create result
    char pad_010[0x4];
    int unk_14; // 0x14: cleared by the constructor
    char pad_018[0x4];
    float unk_1C;         // 0x1C: initialized from kFloat_0_AuxMenu
    unsigned char unk_20; // 0x20: set to 2 by the constructor
    char pad_021[0x3];
};

#define CAUXMENUUI_OFF(f, o) typedef char CAuxMenuUI_off_##f[((int)&((CAuxMenuUI*)0)->f == (o)) ? 1 : -1]
CAUXMENUUI_OFF(vtbl, 0x0);
CAUXMENUUI_OFF(firstOverlay, 0x4);
CAUXMENUUI_OFF(secondOverlay, 0x8);
CAUXMENUUI_OFF(thirdOverlay, 0xC);
CAUXMENUUI_OFF(unk_14, 0x14);
CAUXMENUUI_OFF(unk_1C, 0x1C);
CAUXMENUUI_OFF(unk_20, 0x20);
#undef CAUXMENUUI_OFF
typedef char CAuxMenuUI_size[(sizeof(CAuxMenuUI) == 0x24) ? 1 : -1];

#endif // CLASSES_CAUXMENUUI_H
