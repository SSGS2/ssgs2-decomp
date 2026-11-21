// Layout used by the methods that reference it.
#ifndef CLASSES_CPANGYACOMMONDIALOGFIELDS_H
#define CLASSES_CPANGYACOMMONDIALOGFIELDS_H

struct CPangyaCommonDialogFields {
    void* vtbl; // 0x0: CPangyaCommonDialog constructor and vfuncs
    int kind; // 0x4: CPangyaCommonDialog constructor and vfuncs
    int result; // 0x8: CPangyaCommonDialog constructor and vfuncs
    int inputDelay; // 0xc: CPangyaCommonDialog constructor and vfuncs
    void* layout; // 0x10: CPangyaCommonDialog constructor and vfuncs
    int backgroundEntryId; // 0x14: CPangyaCommonDialog constructor and vfuncs
    int dialogEntryId; // 0x18: CPangyaCommonDialog constructor and vfuncs
    void* message; // 0x1c: CPangyaCommonDialog constructor and vfuncs
    void* tagExpander; // 0x20: CPangyaCommonDialog constructor and vfuncs
    unsigned int textFlags; // 0x24: CPangyaCommonDialog constructor and vfuncs
    int textLength; // 0x28: CPangyaCommonDialog constructor and vfuncs
    void* textData; // 0x2c: CPangyaCommonDialog constructor and vfuncs
    int fading; // 0x30: CPangyaCommonDialog_StartFade and CPangyaCommonDialog_UpdateFadeAndInput
    int opacity; // 0x34: CPangyaCommonDialog_StartFade and CPangyaCommonDialog_UpdateFadeAndInput
    int opacityStep; // 0x38: CPangyaCommonDialog_StartFade and CPangyaCommonDialog_UpdateFadeAndInput
    int fadeDuration; // 0x3c: CPangyaCommonDialog_StartFade and CPangyaCommonDialog_UpdateFadeAndInput
    int savedScriptIndex; // 0x40: CPangyaCommonDialog_StartFade and CPangyaCommonDialog_UpdateFadeAndInput
};

#define OFF(f, o) typedef char CPangyaCommonDialogFields_off_##f[((int)&((CPangyaCommonDialogFields*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(kind, 0x4);
OFF(result, 0x8);
OFF(inputDelay, 0xc);
OFF(layout, 0x10);
OFF(backgroundEntryId, 0x14);
OFF(dialogEntryId, 0x18);
OFF(message, 0x1c);
OFF(tagExpander, 0x20);
OFF(textFlags, 0x24);
OFF(textLength, 0x28);
OFF(textData, 0x2c);
OFF(fading, 0x30);
OFF(opacity, 0x34);
OFF(opacityStep, 0x38);
OFF(fadeDuration, 0x3c);
OFF(savedScriptIndex, 0x40);
#undef OFF
#define AsCPangyaCommonDialogFields(p) ((CPangyaCommonDialogFields*)(p))

#endif
