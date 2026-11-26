// The tour area-selection menu: the BP scene entries, the two layouts, the
// archive and texture resource it loads, and the queue of areas still to reveal.
#ifndef CLASSES_MENUGROUP802FB4D0_H
#define CLASSES_MENUGROUP802FB4D0_H

#include "game/types.h"

struct AreaRevealNode;

class MenuGroup802FB4D0 {
public:
    // Advance the resource load, area reveals, reward dispatch and selection.
    // Returns 0 while pending, -1 when cancelled and +1 once confirmed.
    int ProcessAreaSelection(float deltaSeconds);

    union {
        struct {
            int selectedArea;     // 0x0: -1 when no area is hovered
            int controllerIds[3]; // 0x4: BP scene entries
        };
        struct {
            char pad_000[8];
            int bpEntryId; // 0x8: second scene entry, which owns the level group
            char pad_00C[4];
        };
    };
    int entryCount;               // 0x10: number of valid controllerIds
    int state;                    // 0x14: area-selection state
    void* firstLayout;            // 0x18
    void* secondLayout;           // 0x1C: the area-detail layout
    void* packedFile;             // 0x20
    void* textureResource;        // 0x24
    char pad_028[4];              // 0x28
    int remaining;                // 0x2C: areas left to reveal
    void* treeRoot;               // 0x30
    AreaRevealNode* revealCursor; // 0x34
};

#define MENUGROUP802FB4D0_OFF(f, o) \
    typedef char MenuGroup802FB4D0_off_##f[((int)&((MenuGroup802FB4D0*)0)->f == (o)) ? 1 : -1]
MENUGROUP802FB4D0_OFF(selectedArea, 0x0);
MENUGROUP802FB4D0_OFF(controllerIds, 0x4);
MENUGROUP802FB4D0_OFF(bpEntryId, 0x8);
MENUGROUP802FB4D0_OFF(entryCount, 0x10);
MENUGROUP802FB4D0_OFF(state, 0x14);
MENUGROUP802FB4D0_OFF(firstLayout, 0x18);
MENUGROUP802FB4D0_OFF(secondLayout, 0x1c);
MENUGROUP802FB4D0_OFF(packedFile, 0x20);
MENUGROUP802FB4D0_OFF(textureResource, 0x24);
MENUGROUP802FB4D0_OFF(remaining, 0x2c);
MENUGROUP802FB4D0_OFF(treeRoot, 0x30);
MENUGROUP802FB4D0_OFF(revealCursor, 0x34);
#undef MENUGROUP802FB4D0_OFF
typedef char MenuGroup802FB4D0_size[(sizeof(MenuGroup802FB4D0) == 0x38) ? 1 : -1];

#endif // CLASSES_MENUGROUP802FB4D0_H
