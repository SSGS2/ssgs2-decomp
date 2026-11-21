// IDiskErrorDialog: layout used by IDiskErrorDialog_StartFadeOut, IDiskErrorDialog_StartFadeIn and IDiskErrorDialog_InitFade/6.
// Data-only declaration; no compiler-generated vtable or RTTI.
#ifndef CLASSES_IDISKERRORDIALOG_H
#define CLASSES_IDISKERRORDIALOG_H
class IDiskErrorDialog {
public:
    void* vtbl;                         // 0x0: IDiskErrorDialog_dtor installs __vt__16IDiskErrorDialog
    char tween[0x1C];                   // 0x4: embedded object passed to FloatTween_Start
    float tweenDuration;                      // 0x20: IDiskErrorDialog_StartFadeOut/E8 forward as second float
};
#define IDISKERRORDIALOG_OFF(f, o) typedef char IDiskErrorDialog_off_##f[((int)&((IDiskErrorDialog*)0)->f == (o)) ? 1 : -1]
IDISKERRORDIALOG_OFF(vtbl, 0x0);
IDISKERRORDIALOG_OFF(tween, 0x4);
IDISKERRORDIALOG_OFF(tweenDuration, 0x20);
#undef IDISKERRORDIALOG_OFF
#endif
