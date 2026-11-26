// CMiniGameAim: methods in minigame_aim_and_party_tiers.cpp; plain layout view, no emitted vtable.
#ifndef CLASSES_CMINIGAMEAIM_H
#define CLASSES_CMINIGAMEAIM_H

class CMiniGameAim {
public:
    void* table;         // 0x0: ResolveEntries loads the file buffer; GetEntry calls Table_GetEntry
    void* vtable;        // 0x4: constructor and destructor
    void* sharedFile;    // 0x8: PollLoad creates; destructor calls CSharedFileHandle_Destroy
    void* fallbackEntry; // 0xC: ResolveEntries allocates three ints; GetEntry returns it

    int vfunc_1();
    int vfunc_2();
    int vfunc_3();
    int vfunc_4();
    void* GetEntry(int index);
    int GetEntryCount();
    void ResolveEntries();
    int PollLoad(int fileName);

    CMiniGameAim();
    ~CMiniGameAim();
};

#define OFF(f, o) typedef char CMiniGameAim_off_##f[((int)&((CMiniGameAim*)0)->f == (o)) ? 1 : -1]
OFF(table, 0x0);
OFF(vtable, 0x4);
OFF(sharedFile, 0x8);
OFF(fallbackEntry, 0xC);
#undef OFF
#endif
