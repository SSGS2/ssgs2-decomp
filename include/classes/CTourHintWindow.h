// CTourHintWindow: the tour hint window singleton. It holds four message renderers,
// the two BP layout entries of a hint, the window state, and the fade opacity that
// its owner steps up or down.
#ifndef CLASSES_CTOURHINTWINDOW_H
#define CLASSES_CTOURHINTWINDOW_H

#include "classes/CTourSubMessage.h"

// First base: the message object's vtable, navigation flag and text color.
struct TourMessageBase {
    void* vtbl;               // 0x00
    int nextIndicatorEnabled; // 0x04
    char pad_08[8];           // 0x08
    int textColor;            // 0x10  starts as -256 (opaque white mask)
    int callbackState;        // 0x14
    TourMessageBase();
    ~TourMessageBase();
};

class CTourHintWindow : public TourMessageBase, public CTourSubMessage {
public:
    CTourHintWindow();

    // Fade. Each update moves the opacity by fadeStep toward fully transparent or opaque.
    void StartFadeIn();
    void StartFadeOut();
    void SetFadeStep(unsigned char value);
    // The caller passes a float that this does not read.
    int UpdateFade(float);

    // BP layout entries and their resources.
    void RecreateLayoutEntries();
    bool AreResourcesLoaded();
    void* ReleaseSingletonResources(int flags);

    // Window state.
    void SetState(int value);
    int IsState(int value);

    // Draw, update and destroy the shared window. The singleton is used, not the receiver.
    void DrawCurrentLayout();
    // The caller passes a message holder that this does not read.
    static void DrawSingletonPage(void* messageHolder);
    int UpdateNextIndicatorAndCallback();
    // Entered with this 0x18 past the window: steps back by 0x18 and runs the destructor.
    void DtorThunk18(int deleting);

    // Hint pages: page 0 is the caddie script, page 1 the Pipin script.
    void SelectPage(signed char page);
    void ResetPageScripts();
    void SetNavigationOpacity(int opacity);
    void DrawCurrentPage();
    void ClearRegisteredObjects();

    int fadeState;    // 0x40  4 decreases opacity, 5 increases it, 3 means fade complete
    int state;        // 0x44
    int layoutIds[2]; // 0x48  BP entries for the two hint resources, -1 when none
    // The hint-window code also uses mode as the page index into layoutIds.
    int mode;                // 0x50  0 touches glyphs only, 1 also the BP groups
    unsigned char opacity;   // 0x54
    char pad_55;             // 0x55
    unsigned char fadeStep;  // 0x56
    char pad_57;             // 0x57
    int active;              // 0x58
    char pad_5C[4];          // 0x5C
    int updateNextIndicator; // 0x60  nonzero when the next indicator is to be set
};

#define CTOURHINTWINDOW_OFFSET(field, offset) \
    typedef char CTourHintWindow_offset_##field[((int)&((CTourHintWindow*)0)->field == (offset)) ? 1 : -1]
CTOURHINTWINDOW_OFFSET(vtbl, 0x00);
CTOURHINTWINDOW_OFFSET(nextIndicatorEnabled, 0x04);
CTOURHINTWINDOW_OFFSET(textColor, 0x10);
CTOURHINTWINDOW_OFFSET(items, 0x20);
CTOURHINTWINDOW_OFFSET(baseGlyphColor, 0x34);
CTOURHINTWINDOW_OFFSET(fadeState, 0x40);
CTOURHINTWINDOW_OFFSET(state, 0x44);
CTOURHINTWINDOW_OFFSET(layoutIds, 0x48);
CTOURHINTWINDOW_OFFSET(mode, 0x50);
CTOURHINTWINDOW_OFFSET(opacity, 0x54);
CTOURHINTWINDOW_OFFSET(fadeStep, 0x56);
CTOURHINTWINDOW_OFFSET(active, 0x58);
CTOURHINTWINDOW_OFFSET(updateNextIndicator, 0x60);
#undef CTOURHINTWINDOW_OFFSET
typedef char CTourHintWindow_size[(sizeof(CTourHintWindow) == 0x64) ? 1 : -1];

#endif // CLASSES_CTOURHINTWINDOW_H
