// MPBase1: methods owned by event_face_and_mpbase1_menu.cpp. A layout view without virtual declarations.
// MPBase1_Process (0x802419E8) initializes once, starts, and records completion.
#ifndef CLASSES_MPBASE1LAYOUT_H
#define CLASSES_MPBASE1LAYOUT_H
struct MPBase1Layout {
    void* vtable;           // 0x0: MPBase1_Ctor (0x80241964)
    int running;            // 0x4: MPBase1_Start sets after m3; Process clears after completion
    int initialized;        // 0x8: Process sets after m2; ReleaseIfInitialized tests
    int autoStart;          // 0xC: Process gates m3 after initialization; MPBase1_SetAutoStart setter
    int unk_10;             // 0x10: MPBase1_Ctor clears
    unsigned char complete; // 0x14: Process returns early when set; MPBase1_IsComplete getter
};
#define OFF(f, o) typedef char MPBase1Layout_off_##f[((int)&((MPBase1Layout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(running, 0x4);
OFF(initialized, 0x8);
OFF(autoStart, 0xC);
OFF(unk_10, 0x10);
OFF(complete, 0x14);
#undef OFF
#define AsMPBase1(p) ((MPBase1Layout*)(p))
#endif
