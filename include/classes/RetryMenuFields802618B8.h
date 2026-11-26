// CRetryMenu view, owned by retry_menu.cpp. Constructor (0x802618D0) saves mode and allocated layout; vfunc_5 (0x80261AA0) assigns selectedResult and decrements selectionDelayUpdates. Separate view preserves the existing constructor class.
#ifndef CLASSES_RETRYMENUFIELDS802618B8_H
#define CLASSES_RETRYMENUFIELDS802618B8_H

struct RetryMenuView {
    char base[0x18];
    void* layout;
    int mode;
    int selectedResult;
    int selectionDelayUpdates;
};

#define OFF(f, o) typedef char RetryMenuView_off_##f[((int)&((RetryMenuView*)0)->f == (o)) ? 1 : -1]
OFF(base, 0x0);
OFF(layout, 0x18);
OFF(mode, 0x1c);
OFF(selectedResult, 0x20);
OFF(selectionDelayUpdates, 0x24);
#undef OFF

#endif
