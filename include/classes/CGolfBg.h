// CGolfBg: partial layout view; CGolfBg_HandleMsg and reload helpers in golf_bg_course_reload_and_camera.cpp.
// No inheritance, virtual declarations, or emitted runtime objects.
#ifndef CLASSES_CGOLFBG_H
#define CLASSES_CGOLFBG_H
class CGolfBg {
public:
    char pad_000[0x18];
    int courseReloadSettled; // 0x18: Nonzero means no started reload awaits finalization; zero while a new load is pending.
    unsigned char savedHoleIndex; // 0x1C: CGolfBg_RequestCourseReload saves currentHoleIndex; CGolfBg_FinalizeCourseReload restores it.
    unsigned char savedHoleCount; // 0x1D: same helpers save/restore roundHoleCount; zero means no saved round.
    unsigned char savedHoleOrder[18]; // 0x1E: copied to/from shared document +0x82A in three groups of six.
};
#define OFF(f, o) typedef char CGolfBg_off_##f[((int)&((CGolfBg*)0)->f == (o)) ? 1 : -1]
OFF(courseReloadSettled, 0x18);
OFF(savedHoleIndex, 0x1C);
OFF(savedHoleCount, 0x1D);
OFF(savedHoleOrder, 0x1E);
#undef OFF
#define AsCGolfBg(p) ((CGolfBg*)(p))
#endif
