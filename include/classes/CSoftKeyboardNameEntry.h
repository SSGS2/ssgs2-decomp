#ifndef CLASSES_CSOFTKEYBOARDNAMEENTRY_H
#define CLASSES_CSOFTKEYBOARDNAMEENTRY_H

#include "game/types.h"

// The data layout of CSoftKeyboardNameEntry has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets. The fields below are the ones its destructor uses.
class CSoftKeyboardNameEntry {
public:
    int vfunc_5();
    bool IsReady();
    int InitPanelAndLayouts();
    int UpdateLayoutsAndInput();
    int DrawKeyboardAndText();

    unsigned int AppendEntry(void* value);

    ~CSoftKeyboardNameEntry();

    void* vtbl;            // 0x00
    char pad_04[0xAC];     // 0x04
    int uiEntryId;         // 0xB0
    char pad_B4[0x10];     // 0xB4
    void* firstGlyphPage;  // 0xC4
    void* secondGlyphPage; // 0xC8
    void* nameLayout;      // 0xCC
    void* cursorLayout;    // 0xD0
};

#endif // CLASSES_CSOFTKEYBOARDNAMEENTRY_H
