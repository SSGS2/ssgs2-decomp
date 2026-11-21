// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_MENUOWNER802FB7F8_H
#define CLASSES_MENUOWNER802FB7F8_H

#include "game/types.h"

class MenuOwner802FB7F8 {
public:
    int screenState;                          // 0x0: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    int bpEntryId;                      // 0x4: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    void* layout;                       // 0x8: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    unsigned char selectedCharacterIndex;                // 0xc: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    unsigned char selectedCaddieIndex;                // 0xd: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    char pad_00E[0x12];                 // 0xe
    void* context;                      // 0x20: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
    void* registeredObject;             // 0x24: TourItemMenu_Init initializes; CTourWindow_dtor destroys layout and deletes BP entry; TourItemMenu_DisableRegisteredObject uses registeredObject
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define MENUOWNER802FB7F8_OFF(f, o) typedef char MenuOwner802FB7F8_off_##f[((int)&((MenuOwner802FB7F8*)0)->f == (o)) ? 1 : -1]
MENUOWNER802FB7F8_OFF(bpEntryId, 0x4);
MENUOWNER802FB7F8_OFF(layout, 0x8);
MENUOWNER802FB7F8_OFF(selectedCharacterIndex, 0xc);
MENUOWNER802FB7F8_OFF(selectedCaddieIndex, 0xd);
MENUOWNER802FB7F8_OFF(context, 0x20);
MENUOWNER802FB7F8_OFF(registeredObject, 0x24);
MENUOWNER802FB7F8_OFF(screenState, 0x0);
#undef MENUOWNER802FB7F8_OFF

#endif  // CLASSES_MENUOWNER802FB7F8_H
