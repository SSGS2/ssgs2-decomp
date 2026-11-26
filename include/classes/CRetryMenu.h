// CRetryMenu: the choice shown after a round. It owns an 18-element layout,
// keeps the mode it was created with, and records the choice the player makes.
#ifndef CLASSES_CRETRYMENU_H
#define CLASSES_CRETRYMENU_H

extern "C" void MPBase1_Ctor(void*);

// The menu-process base of the retry menu. Its constructor calls MPBase1_Ctor.
// The destructor keeps this ABI spelling: the constructor's unwind cleanup refers to it.
struct CWarningLogoBase {
    char data[0x18];
    CWarningLogoBase() {
        MPBase1_Ctor(this);
    }
    ~CWarningLogoBase();
};

class CRetryMenu : public CWarningLogoBase {
public:
    CRetryMenu(int menuMode);

    // Release the layout and restore hit testing. shouldDelete > 0 also deletes
    // the object, so the returned pointer may then be dangling.
    void* Destroy(int shouldDelete);

    // Advance the layout loading and animation work.
    int UpdateLayout();
    // Report whether the layout has finished loading.
    int PollLoadComplete();
    // Hide unavailable choices for this mode and reset the completion countdown.
    void ResetSelectionState();
    // Apply the activated choice and count down to the end of the menu.
    // Returns 1 only when a started countdown reaches zero.
    int UpdateSelection();
    // Draw the retry background quad and its layout at full opacity.
    void Draw();

    void* layout;              // 0x18
    int mode;                  // 0x1C
    int selectedResult;        // 0x20  the choice made, not an animation phase
    int selectionDelayUpdates; // 0x24  counts update calls after a choice
};

#define CRETRYMENU_OFFSET(field, offset) \
    typedef char CRetryMenu_offset_##field[((int)&((CRetryMenu*)0)->field == (offset)) ? 1 : -1]
CRETRYMENU_OFFSET(layout, 0x18);
CRETRYMENU_OFFSET(mode, 0x1C);
CRETRYMENU_OFFSET(selectedResult, 0x20);
CRETRYMENU_OFFSET(selectionDelayUpdates, 0x24);
#undef CRETRYMENU_OFFSET
typedef char CRetryMenu_size[(sizeof(CRetryMenu) == 0x28) ? 1 : -1];

#endif // CLASSES_CRETRYMENU_H
