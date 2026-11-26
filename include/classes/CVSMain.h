// CVSMain: the versus-mode main task object.
//
// It shares its layout with CPartyMain and holds the secondary interface's
// vtable pointer, the active selection screen, and the two party item vectors.
#ifndef CLASSES_CVSMAIN_H
#define CLASSES_CVSMAIN_H

#include "game/types.h"

// A task message: the command type follows one unused word.
struct CVSMainMessage {
    int unused; // 0x00
    int type;   // 0x04
};

class CVSMain {
public:
    CVSMain();
    ~CVSMain();

    static void OnPreLoadInit();

    // Replace the owned selection screen or toggle both BP layers from a task
    // message.
    void HandleMsg(const CVSMainMessage* message);

    // Destructor entry of the secondary interface's vtable: that pointer is
    // 0x18 bytes past the object's start.
    void DestroySecondaryInterface(int deleteFlag);

    void* vtbl;               // 0x00  original vtable pointer
    char pad_004[0x14];       // 0x04
    void* taskMainVtable;     // 0x18
    int state;                // 0x1C  active selection state
    void* selectionScreen;    // 0x20  the one owned screen, or null
    void* primaryBpEntryId;   // 0x24  background BP entry
    void* secondaryBpEntryId; // 0x28  caption BP entry
    char pad_02C[0x08];       // 0x2C
    void* primaryItems;       // 0x34
    int primaryItemCount;     // 0x38
    char pad_03C[0x4];        // 0x3C
    void* secondaryItems;     // 0x40
    int secondaryItemCount;   // 0x44
};

#define CVSMAIN_OFF(f, o) typedef char CVSMain_off_##f[((int)&((CVSMain*)0)->f == (o)) ? 1 : -1]
CVSMAIN_OFF(taskMainVtable, 0x18);
CVSMAIN_OFF(state, 0x1C);
CVSMAIN_OFF(selectionScreen, 0x20);
CVSMAIN_OFF(primaryBpEntryId, 0x24);
CVSMAIN_OFF(secondaryBpEntryId, 0x28);
CVSMAIN_OFF(primaryItems, 0x34);
CVSMAIN_OFF(primaryItemCount, 0x38);
CVSMAIN_OFF(secondaryItems, 0x40);
CVSMAIN_OFF(secondaryItemCount, 0x44);
#undef CVSMAIN_OFF

#endif // CLASSES_CVSMAIN_H
