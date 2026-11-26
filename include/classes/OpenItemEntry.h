// OpenItemEntry: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMENTRY_H
#define CLASSES_OPENITEMENTRY_H

class OpenItemEntry {
public:
    void* record; // 0x0: OpenItemMenu_UpdateEntryVisuals
    short index;  // 0x4: OpenItemMenu_BuildEntries, OpenItemMenu_UpdateEntryVisuals, CTourMain_HandleItemMapNavigation
};

#define OFF(f, o) typedef char OpenItemEntry_off_##f[((int)&((OpenItemEntry*)0)->f == (o)) ? 1 : -1]
OFF(record, 0x0);
OFF(index, 0x4);
#undef OFF

#endif
