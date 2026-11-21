// CMiniGameAim: methods in minigame_aim_and_party_tiers.cpp; plain layout view, no emitted vtable.
#ifndef CLASSES_CMINIGAMEAIM_H
#define CLASSES_CMINIGAMEAIM_H

class CMiniGameAim {
public:
    void* table; // 0x0: CMiniGameAim_ResolveEntries loads file buffer; CMiniGameAim_GetEntry calls Table_GetEntry
    void* vtable; // 0x4: CMiniGameAim_Ctor and CMiniGameAim_dtor
    void* sharedFile; // 0x8: CMiniGameAim_PollLoad creates; dtor calls CSharedFileHandle_Destroy
    void* fallbackEntry; // 0xC: CMiniGameAim_ResolveEntries allocates three ints; CMiniGameAim_GetEntry returns it
};

#define OFF(f, o) typedef char CMiniGameAim_off_##f[((int)&((CMiniGameAim*)0)->f == (o)) ? 1 : -1]
OFF(table, 0x0);
OFF(vtable, 0x4);
OFF(sharedFile, 0x8);
OFF(fallbackEntry, 0xC);
#undef OFF
#endif
