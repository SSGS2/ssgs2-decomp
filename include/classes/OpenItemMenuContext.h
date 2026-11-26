// OpenItemMenuContext: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMMENUCONTEXT_H
#define CLASSES_OPENITEMMENUCONTEXT_H

class OpenItemMenuContext {
public:
    void* saveRecord;    // 0x0: CTourMain_ProcessItemMenu
    unsigned char event; // 0x4: OpenItemMenu_UpdateEntryVisuals
    char pad_005[0x3];
    int eventValue; // 0x8: OpenItemMenu_UpdateEntryVisuals
    char pad_00C[0x8];
    char itemEvent; // 0x14: CTourMain_ProcessItemMenu
    char pad_015[0xf];
    int nextState; // 0x24: CTourMain_ProcessItemMenu
    char pad_028[0x8];
    int linkActive;      // 0x30: CTourMain_ProcessItemMenu
    int useDefaultNames; // 0x34: CTourMain_ProcessItemMenu, CTourMain_PrepareSelectedEncounter
};

#define OFF(f, o) typedef char OpenItemMenuContext_off_##f[((int)&((OpenItemMenuContext*)0)->f == (o)) ? 1 : -1]
OFF(saveRecord, 0x0);
OFF(event, 0x4);
OFF(eventValue, 0x8);
OFF(itemEvent, 0x14);
OFF(nextState, 0x24);
OFF(linkActive, 0x30);
OFF(useDefaultNames, 0x34);
#undef OFF

#endif
