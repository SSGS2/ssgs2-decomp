// CTourVSWindow: the tour versus window.
//
// Two player banners with name strips, and the panels that expand, settle and
// fade out between them. The layout is TourVSWindowData. The window's vtable is
// taken from the original game data, so nothing is declared virtual here.
#ifndef CLASSES_CTOURVSWINDOW_H
#define CLASSES_CTOURVSWINDOW_H

#include "game/types.h"
#include "classes/TourVSWindowData.h"

class CTourVSWindow : public TourVSWindowData {
public:
    CTourVSWindow();

    // Entry indices and the hidden state.
    void SetEntryIndices(int playerOneIndex, int playerTwoIndex);
    void ResetAndHide();

    // Transition and group opacity.
    void BeginTransition();
    void SetGroupOpacity(void* opacity);

    // Layout: resets the BP layout, name strips and transition geometry.
    void InitLayout(short mode);

    // Expands and settles the panels; returns 1 on the update that completes the script.
    int UpdateTransition(float deltaTime);

    // Draws the banners, panels and name layers, then the BP overlay.
    void Draw();

    // Global window: the first three act on the shared instance, and the object they are
    // called on is not used. Dtor hides the shared instance and frees the object it is called on.
    void DrawGlobal();
    bool IsReady();
    void LoadUIResources();
    void* Dtor(int destroy);
    void* Destroy(int deleting); // releases the object's banners, BP entry and fade state

    // Message mode: sets the player name strip, message colour and source frames.
    void SetMessageMode(int mode, struct TourMessageText* message);

    int vfunc_1();
};

#define CTOURVSWINDOW_OFFSET(field, offset) \
    typedef char CTourVSWindow_offset_##field[((int)&((CTourVSWindow*)0)->field == (offset)) ? 1 : -1]
CTOURVSWINDOW_OFFSET(vt, 0x00);
CTOURVSWINDOW_OFFSET(opacity, 0x1C);
#undef CTOURVSWINDOW_OFFSET
typedef char CTourVSWindow_size[(sizeof(CTourVSWindow) == 0x64) ? 1 : -1];

#endif // CLASSES_CTOURVSWINDOW_H
