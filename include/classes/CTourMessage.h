// Partial layout view owned by tour_hint_message_window.cpp; no constructors, vtable emission or RTTI.
#ifndef CLASSES_CTOURMESSAGE_H
#define CLASSES_CTOURMESSAGE_H
struct CTourMessage {
    char pad_0[0x40];
    int fadeState; // 0x802FE92C: 4 decreases opacity, 5 increases it, 3 means fade complete.
};
#define OFF(f, o) typedef char CTourMessage_off_##f[((int)&((CTourMessage*)0)->f == (o)) ? 1 : -1]
OFF(fadeState, 0x40);
#undef OFF
#endif
