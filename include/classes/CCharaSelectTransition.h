// CCharaSelect_UpdateAndPollFinished (0x80243D38), chara_select_screen_part2.cpp: four dynamically allocated 12-byte records.
#ifndef CLASSES_CCHARASELECTTRANSITION_H
#define CLASSES_CCHARASELECTTRANSITION_H
struct CCharaSelectTransition {
    int active;           // Set on activated elements 42..49; cleared at update tick 10.
    int updateTick;       // Incremented while active; 11 wraps to 0, 10 completes transition.
    int activatedElement; // Layout_CollectActivated code; selects paired element enable/disable calls.
};
#define OFF(f, o) typedef char CCharaSelectTransition_off_##f[((int)&((CCharaSelectTransition*)0)->f == (o)) ? 1 : -1]
OFF(active, 0x0);
OFF(updateTick, 0x4);
OFF(activatedElement, 0x8);
#undef OFF
typedef char CCharaSelectTransition_size[(sizeof(CCharaSelectTransition) == 12) ? 1 : -1];
#endif
