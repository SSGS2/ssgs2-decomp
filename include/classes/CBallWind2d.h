// CBallWind2d: the in-round HUD panel for ball selection and wind.
//
// It is a layout of 30 numbered elements. The class keeps one cached flag or
// value per group of elements; ApplyState pushes them into the layout once the
// layout has been initialised.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so the vtable pointers are spelled
// out as members. The methods below that are called through the vtable are the
// virtual slots of the HUD interface.
#ifndef CLASSES_CBALLWIND2D_H
#define CLASSES_CBALLWIND2D_H

#include "game/types.h"

class CBallWind2d {
public:
    // Update and drawing.
    int Update();
    int Draw(void* ballObject);

    // Ball selector.
    int SelectElementForValue(int value);
    int SetSelectionActive(int active);
    int SetSelectionState(int state);
    int IsSelectorActivated();
    int PlayClickSounds();

    // Which element groups are shown.
    int SetBallControlsVisible(int visible);
    int IsBallControlsVisible();
    int SetElement0Visible(int visible);
    int SetElements26And27Visible(int visible);
    int SetElements22And25Visible(int visible);
    int SetElement22Visible(int visible);
    int SetElements25And26Bright(int bright);
    int SetElement28Visible(int visible);
    int SetElement29Visible(int visible);
    int SetElement23Visible(int visible);
    int SetElement24Visible(int visible);

    // Indicator elements 1 and 3.
    int SetElement3DrawnSeparately(int separate);
    int SetElement1DrawnSeparately(int separate);
    int SetIndicatorCenterX(float centerX);
    int SetIndicatorColor(int color);

    // Virtual slot 24: always accepts.
    int vfunc_24();

    // Push the cached state into the layout.
    int ApplyState();

    void* vtbl;                  // 0x00
    char pad_004[0x58];          // 0x04  layout base object
    int selectedElementIndex;    // 0x5C  layout element enabled with the selector
    int selectionActive;         // 0x60
    int selectionState;          // 0x64  0, 1 or 2
    int ballControlsVisible;     // 0x68
    int element0Visible;         // 0x6C
    int elements26And27Visible;  // 0x70
    int element22Visible;        // 0x74
    int element25Visible;        // 0x78
    int elements25And26Bright;   // 0x7C
    int element23Visible;        // 0x80
    int element24Visible;        // 0x84
    int element28Visible;        // 0x88  excludes element 29
    int element29Visible;        // 0x8C  excludes element 28
    int element3DrawnSeparately; // 0x90
    int element1DrawnSeparately; // 0x94
    float indicatorCenterX;      // 0x98  elements 1 and 3 are centred on this x
    int indicatorColor;          // 0x9C
    int layoutReady;             // 0xA0  set once the layout is initialised
    void* interfaceVtable;       // 0xA4  vtable of the HUD interface

    int InitLayout();
    static int LoadLayout(void* arg0, void* arg1);

    CBallWind2d();
    ~CBallWind2d();
};

#define CBALLWIND2D_OFFSET(field, offset) \
    typedef char CBallWind2d_offset_##field[((int)&((CBallWind2d*)0)->field == (offset)) ? 1 : -1]
CBALLWIND2D_OFFSET(selectedElementIndex, 0x5C);
CBALLWIND2D_OFFSET(ballControlsVisible, 0x68);
CBALLWIND2D_OFFSET(element28Visible, 0x88);
CBALLWIND2D_OFFSET(element1DrawnSeparately, 0x94);
CBALLWIND2D_OFFSET(indicatorCenterX, 0x98);
CBALLWIND2D_OFFSET(layoutReady, 0xA0);
CBALLWIND2D_OFFSET(interfaceVtable, 0xA4);
#undef CBALLWIND2D_OFFSET
typedef char CBallWind2d_size[(sizeof(CBallWind2d) == 0xA8) ? 1 : -1];

#endif // CLASSES_CBALLWIND2D_H
