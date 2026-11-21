// CAuxMenuUILayout: non-virtual layout view; methods/accesses in head_icon_and_aux_menu_ui.cpp.
// No inheritance or virtual declarations: the original vtables remain external symbols.
#ifndef CLASSES_CAUXMENUUILAYOUT_H
#define CLASSES_CAUXMENUUILAYOUT_H

class CAuxMenuUILayout {
public:
    void* vtbl; // 0x0: CAuxMenuUI constructor/dtor: __vt__10CAuxMenuUI
    void* firstOverlay; // 0x4: CAuxMenuUI constructor: first WOverlay_Create result
    void* secondOverlay; // 0x8: CAuxMenuUI constructor: second WOverlay_Create result
    void* thirdOverlay; // 0xC: CAuxMenuUI constructor: third WOverlay_Create result
    char pad_010[0x4];
    int unk_14; // 0x14: CAuxMenuUI constructor: cleared
    char pad_018[0x4];
    float unk_1C; // 0x1C: CAuxMenuUI constructor: initialized from kFloat_0_AuxMenu
    unsigned char unk_20; // 0x20: CAuxMenuUI constructor: set to 2
};

#define OFF(f, o) typedef char CAuxMenuUILayout_off_##f[((int)&((CAuxMenuUILayout*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(firstOverlay, 0x4);
OFF(secondOverlay, 0x8);
OFF(thirdOverlay, 0xC);
OFF(unk_14, 0x14);
OFF(unk_1C, 0x1C);
OFF(unk_20, 0x20);
#undef OFF

#endif
