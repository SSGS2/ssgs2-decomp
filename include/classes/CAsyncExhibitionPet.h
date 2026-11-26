// CAsyncExhibitionPet: a preview that shows a pet item.
//
// It adds the selected pet item, the latch for a selection that changed while
// the current pet was loading, and the pet's own preloader to the shared
// exhibition state.
#ifndef CLASSES_CASYNCEXHIBITIONPET_H
#define CLASSES_CASYNCEXHIBITIONPET_H

#include "game/types.h"

class CAsyncExhibitionPet {
public:
    // Allocate a preview for the given viewport, or return null if it cannot be
    // initialized.
    static CAsyncExhibitionPet* Create(void* viewport, int mode, float scale);

    int UpdateAndInstallPendingPet(float delta);
    int SetPetSelection(void* value);
    int ApplyPetSelection();
    void* CreatePetPreload();
    int RestartPetPreload();
    unsigned char vfunc_8();

    ~CAsyncExhibitionPet();

    char pad_000[0x124];         // 0x000  CAsyncExhibition state
    void* petSelection;          // 0x124  item ID of the pet being shown
    unsigned char updatePending; // 0x128  a selection changed while the current pet loads
    char pad_129[3];
    void* petPreload; // 0x12C
};

#define OFF(f, o) typedef char CAsyncExhibitionPet_off_##f[((int)&((CAsyncExhibitionPet*)0)->f == (o)) ? 1 : -1]
OFF(petSelection, 0x124);
OFF(updatePending, 0x128);
OFF(petPreload, 0x12C);
#undef OFF
typedef char CAsyncExhibitionPet_size[(sizeof(CAsyncExhibitionPet) == 0x130) ? 1 : -1];

#endif
