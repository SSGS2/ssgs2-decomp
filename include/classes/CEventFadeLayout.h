// CEventFadeLayout: layout view for CEventEffectSC_EFF_FADE in event_script_model_commands.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CEVENTFADELAYOUT_H
#define CLASSES_CEVENTFADELAYOUT_H

class CEventFadeLayout {
public:
    void* vtable; // 0x0: constructor installs __vt__23CEventEffectSC_EFF_FADE
    unsigned char direction; // 0x4: vfunc_1 chooses alpha inversion
    char pad_5[0x3];
    unsigned int count; // 0x8: vfunc_1 divides elapsed time and checks completion
};

#define OFF(f, o) typedef char CEventFadeLayout_off_##f[((int)&((CEventFadeLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(direction, 0x4);
OFF(count, 0x8);
#undef OFF

#endif
