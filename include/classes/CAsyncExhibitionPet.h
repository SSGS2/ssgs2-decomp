// CAsyncExhibitionPet: layout view owned by pet_preload_async.cpp.
// No inheritance, virtual declarations, constructors, or emitted code.
#ifndef CLASSES_CASYNCEXHIBITIONPET_H
#define CLASSES_CASYNCEXHIBITIONPET_H

struct CAsyncExhibitionPet {
    char pad_000[0x124];
    void* petSelection; // 0x124: CAsyncExhibitionPet_SetPetSelection stores input; vfunc_7 passes it to CPreloadPetAsync_Create
    unsigned char updatePending; // 0x128: CAsyncExhibitionPet_SetPetSelection sets while child is busy; vfunc_1 clears after CAsyncExhibition_TryInstallPreloadedObject
    char pad_129[3];
    void* petPreload; // 0x12C: vfunc_7 stores CPreloadPetAsync_Create result; vfunc_6 passes it to CPreloadPetAsync_ClearAndRestart
};

#define OFF(f, o) typedef char CAsyncExhibitionPet_off_##f[((int)&((CAsyncExhibitionPet*)0)->f == (o)) ? 1 : -1]
OFF(petSelection, 0x124);
OFF(updatePending, 0x128);
OFF(petPreload, 0x12C);
#undef OFF
#define AsCAsyncExhibitionPet(p) ((CAsyncExhibitionPet*)(p))

#endif
