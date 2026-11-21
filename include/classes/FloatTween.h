// FloatTween: non-virtual layout view owned by unplayable_ball_drop_sequence.cpp.
// FloatTween_Update (0x8029B6E4) interpolates initial to target
// using elapsed / duration; FloatTween_Init initializes the same seven words.
#ifndef CLASSES_FLOATTWEEN_H
#define CLASSES_FLOATTWEEN_H

struct FloatTween {
    int mode;
    float value;
    float initial;
    float target;
    float duration;
    float elapsed;
    int active;
};

#define OFF(f, o) typedef char FloatTween_off_##f[((int)&((FloatTween*)0)->f == (o)) ? 1 : -1]
OFF(mode, 0x0);
OFF(value, 0x4);
OFF(initial, 0x8);
OFF(target, 0xC);
OFF(duration, 0x10);
OFF(elapsed, 0x14);
OFF(active, 0x18);
#undef OFF

#endif
