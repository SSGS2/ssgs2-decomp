// The earlier accessor set of game/singletons.h (includes Sing_805DD570; Sing_805DC688 not yet renamed), kept for
// game/system/directinput_mouse_keyboard.cpp, whose object was proven with this exact list.
// Singleton instance pointers: the game keeps each one as a function-local static of an inline accessor
// (`inline T* Get() { static T* s; if (!s) { ... } return s; }`); mwcc then loads the pointer before the
// prologue saves r31, which an extern pointer cannot do.
// The static of Sing_<addr>() is the symbol "@LOCAL@Sing_<addr>__Fv@s" (config/R2PE9B/symbols.txt names the
// game's pointer so).
// The accessors must always expand (a unit built with -inline noauto or a large caller would call them).
#ifndef GAME_SINGLETONS_H
#define GAME_SINGLETONS_H
#pragma push
#pragma always_inline on

inline void*& GetInputManagerSlot() {
    static void* s;
    return s;
} // lbl_805DC5C8, guard lbl_805DC5CC
inline void*& GetSaveDataSlot() {
    static void* s;
    return s;
} // lbl_805DC5E0, guard lbl_805DC5E4
inline void*& Sing_805DC5F0() {
    static void* s;
    return s;
} // lbl_805DC5F0, guard lbl_805DC5F4
inline void*& Sing_805DC5F8() {
    static void* s;
    return s;
} // lbl_805DC5F8, guard lbl_805DC5FC
inline void*& Sing_805DC608() {
    static void* s;
    return s;
} // lbl_805DC608, guard lbl_805DC60C
inline void*& Sing_805DC610() {
    static void* s;
    return s;
} // lbl_805DC610, guard lbl_805DC614
inline void*& Sing_805DC620() {
    static void* s;
    return s;
} // lbl_805DC620, guard lbl_805DC624
inline void*& Sing_805DC630() {
    static void* s;
    return s;
} // lbl_805DC630, guard lbl_805DC634
inline void*& Sing_805DC648() {
    static void* s;
    return s;
} // lbl_805DC648, guard lbl_805DC64C
inline void*& Sing_805DC650() {
    static void* s;
    return s;
} // lbl_805DC650, guard lbl_805DC654
inline void*& Sing_805DC660() {
    static void* s;
    return s;
} // lbl_805DC660, guard lbl_805DC664
inline void*& Sing_805DC668() {
    static void* s;
    return s;
} // lbl_805DC668, guard lbl_805DC66C
inline void*& Sing_805DC670() {
    static void* s;
    return s;
} // lbl_805DC670, guard lbl_805DC674
inline void*& Sing_805DC678() {
    static void* s;
    return s;
} // lbl_805DC678, guard lbl_805DC67C
inline void*& Sing_805DC680() {
    static void* s;
    return s;
} // lbl_805DC680, guard lbl_805DC684
inline void*& Sing_805DC688() {
    static void* s;
    return s;
} // lbl_805DC688, guard lbl_805DC68C
inline void*& Sing_805DC690() {
    static void* s;
    return s;
} // lbl_805DC690, guard lbl_805DC694
inline void*& Sing_805DC7B0() {
    static void* s;
    return s;
} // lbl_805DC7B0, guard lbl_805DC7B4
inline void*& Sing_805DC820() {
    static void* s;
    return s;
} // lbl_805DC820, guard lbl_805DC824
inline void*& Sing_805DC830() {
    static void* s;
    return s;
} // lbl_805DC830, guard lbl_805DC834
inline void*& Sing_805DC930() {
    static void* s;
    return s;
} // lbl_805DC930, guard lbl_805DC934
inline void*& Sing_805DC948() {
    static void* s;
    return s;
} // lbl_805DC948, guard lbl_805DC94C
inline void*& Sing_805DC950() {
    static void* s;
    return s;
} // lbl_805DC950, guard lbl_805DC954
inline void*& Sing_805DC958() {
    static void* s;
    return s;
} // lbl_805DC958, guard lbl_805DC95C
inline void*& Sing_805DC960() {
    static void* s;
    return s;
} // lbl_805DC960, guard lbl_805DC964
inline void*& Sing_805DCB20() {
    static void* s;
    return s;
} // lbl_805DCB20, guard lbl_805DCB24
inline void*& Sing_805DD570() {
    static void* s;
    return s;
} // lbl_805DD570, guard lbl_805DD574
inline void*& Sing_805DD688() {
    static void* s;
    return s;
} // lbl_805DD688, guard lbl_805DD68C
inline void*& Sing_805DD690() {
    static void* s;
    return s;
} // lbl_805DD690, guard lbl_805DD694
inline void*& Sing_805DD698() {
    static void* s;
    return s;
} // lbl_805DD698, guard lbl_805DD69C
inline void*& Sing_805DD6A0() {
    static void* s;
    return s;
} // lbl_805DD6A0, guard lbl_805DD6A4

#pragma pop
#endif
