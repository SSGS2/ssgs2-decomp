// CFightMain: the versus-mode main object.
//
// It is constructed over a CVSMain instance and replaces the two interface
// pointers the versus task manager uses. Its message handler builds the stroke
// or match selection menu and keeps the shared player records ready for play.
//
// The interface pointers are spelled out as data because the vtables come from
// the original game data, so the class declares no virtual functions.
#ifndef CLASSES_CFIGHTMAIN_H
#define CLASSES_CFIGHTMAIN_H

#include "game/types.h"
#include "classes/CVSMain.h"

struct FightEvent;

class CFightMain : public CVSMain {
public:
    CFightMain();
    ~CFightMain();

    void HandleMsg(const FightEvent* event);
    static void* GetRTTI();

    // Destructor entry for the secondary interface's vtable: its this pointer is
    // 0x18 bytes past the object's start.
    static void* DestroyThunkMinus24(void* p, int deleting);

    char pad_048[0x4]; // 0x48
};

typedef char CFightMain_size[(sizeof(CFightMain) == 0x4C) ? 1 : -1];

#endif // CLASSES_CFIGHTMAIN_H
