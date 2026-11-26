// ShadowSelection801F: layout view from CShadowManager_ReplaceSelection / CShadowManager_SetSelectionValue / CShadowManager_SetSelectionFlags.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_SHADOWSELECTION801F_H
#define CLASSES_SHADOWSELECTION801F_H

struct ShadowSelection801F {
    void*
        owned; // 0x0: CShadowManager_ReplaceSelection initializes; CShadowManager_ClearEntriesAndResizeCaptureQueue deletes
    char key[0x40]; // 0x4: CShadowManager_ReplaceSelection copies key with size 0x40
    float value;    // 0x44: CShadowManager_SetSelectionValue sets input value
    unsigned int
        flags; // 0x48: CShadowManager_SetSelectionFlags stores byte argument as word; CShadowManager_ReplaceSelection initializes 1
};

#define OFF(f, o) typedef char ShadowSelection801F_off_##f[((int)&((ShadowSelection801F*)0)->f == (o)) ? 1 : -1]
OFF(owned, 0x0);
OFF(key, 0x4);
OFF(value, 0x44);
OFF(flags, 0x48);
#undef OFF
#define AsShadowSelection801F(p) ((ShadowSelection801F*)(p))

#endif
