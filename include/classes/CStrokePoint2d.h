// CStrokePoint2d: the stroke-point selector shown before a shot.
//
// The player drags the strike point away from the ball preview. The selector
// draws the marker, the direction buttons, the power gauge and the special-shot
// buttons, keeps the help message up to date, and reports the chosen direction,
// power and shot to the golf dialog. The class is declared without virtual
// functions on purpose: its interface vtable is taken from the original game
// data, so `interfaceVtable` is spelled out as a member.
#ifndef CLASSES_CSTROKEPOINT2D_H
#define CLASSES_CSTROKEPOINT2D_H

#include "game/types.h"

class CStrokePoint2d {
public:
    // The owner allocates the object and builds it in place with Init.
    CStrokePoint2d* Init();
    ~CStrokePoint2d();

    // Layout, drawing and per-frame update.
    int LoadSpecialShotLayout(int mode);
    int InitLayout();
    int Draw(void* ballPreview);
    int UpdateDialog();

    // Strike point and direction.
    int SetPointPosition(float x, float y);
    int SetDirectionThresholds(float horizontal, float vertical);
    int RestoreSavedSelection();
    int ClearShotSelections();

    // Interaction flags and gauge fill.
    int SetPointerSelectionDisabled(int disabled);
    int SetPowerModeEnabled(int enabled);
    int SetDirectionButtonsEnabled(int enabled);
    int SetExtendedSpecialShotsAllowed(int allowed);
    int SetPowerGaugeFill(int value);

    // Selection state, read through the interface vtable.
    int IsVisible();
    int IsConfirmRequested();
    int GetDirectionSelection();
    int GetSpecialShotSelection();
    int GetPowerMode();
    int ClearDirectionSelection();
    int ClearSpecialShotSelection();
    int ClearPowerMode();

    void* vtbl;                      // 0x00  the layout base's vtable
    char pad_004[0x58];              // 0x04
    float markerOriginX;             // 0x5C  the marker position at zero strike
    float markerOriginY;             // 0x60
    float strikeX;                   // 0x64  the strike offset, in layout units
    float strikeY;                   // 0x68
    float savedStrikeX;              // 0x6C  kept while the dialog is open
    float savedStrikeY;              // 0x70
    float horizontalThreshold;       // 0x74  direction buttons' trigger distance
    float verticalThreshold;         // 0x78
    int inputLock;                   // 0x7C  nonzero disables pointer selection
    int powerModeEnabled;            // 0x80
    int directionButtonsEnabled;     // 0x84
    int extendedSpecialShotsEnabled; // 0x88  Cobra and Spike need a power mode
    int powerGauge;                  // 0x8C  0 to 99, three segments of 33
    int directionSelection;          // 0x90  one of StrokeDirection, or zero
    int savedDirectionSelection;     // 0x94
    int confirmRequested;            // 0x98
    int powerMode;                   // 0x9C  0 to 2 selected power levels
    int savedPowerMode;              // 0xA0
    int specialShotSelection;        // 0xA4
    int savedSpecialShotSelection;   // 0xA8
    int animationFramesRemaining;    // 0xAC  open or close animation
    int dialogState;                 // 0xB0  0 closed, 1 opening, 2 open, 3 closing
    float opacity;                   // 0xB4
    int visible;                     // 0xB8  set from opacity each frame
    int resourcesReady;              // 0xBC  the layout has been initialised
    int controlsDirty;               // 0xC0  the controls need a refresh
    int specialShotOpacity;          // 0xC4  fades the special-shot buttons
    int powerClickFramesRemaining;   // 0xC8
    int powerClickFeedback;          // 0xCC
    float gaugeOriginX;              // 0xD0  the gauge remainder's position
    float gaugeSegmentWidth;         // 0xD4
    void* message;                   // 0xD8  the help message object
    int selectedIndex;               // 0xDC  the selected item, or -1
    void* interfaceVtable;           // 0xE0  the selector's interface vtable

    int HitTestAvailableElement(int arg);
    int HitTestPointerActivation(int arg);
    int UpdateHoveredHelpText();

    int GetCommittedStrikePoint(float* x, float* y);
    int OpenAndSaveSelection();
    int AdvanceDialogAnimation();
    int vfunc_26();
    int StartClosingDialog();
    int IsDialogIdle();
    int RefreshDirtyControls();
    int UpdateStrikeMarkerAndSpinButtons();
    int UpdatePowerAndSpecialShots();
    int ConsumeControlActivations();
};

#define CSTROKEPOINT2D_OFFSET(field, offset) \
    typedef char CStrokePoint2d_offset_##field[((int)&((CStrokePoint2d*)0)->field == (offset)) ? 1 : -1]
CSTROKEPOINT2D_OFFSET(markerOriginX, 0x5C);
CSTROKEPOINT2D_OFFSET(strikeX, 0x64);
CSTROKEPOINT2D_OFFSET(savedStrikeX, 0x6C);
CSTROKEPOINT2D_OFFSET(horizontalThreshold, 0x74);
CSTROKEPOINT2D_OFFSET(inputLock, 0x7C);
CSTROKEPOINT2D_OFFSET(powerGauge, 0x8C);
CSTROKEPOINT2D_OFFSET(directionSelection, 0x90);
CSTROKEPOINT2D_OFFSET(animationFramesRemaining, 0xAC);
CSTROKEPOINT2D_OFFSET(dialogState, 0xB0);
CSTROKEPOINT2D_OFFSET(opacity, 0xB4);
CSTROKEPOINT2D_OFFSET(message, 0xD8);
CSTROKEPOINT2D_OFFSET(selectedIndex, 0xDC);
CSTROKEPOINT2D_OFFSET(interfaceVtable, 0xE0);
#undef CSTROKEPOINT2D_OFFSET
typedef char CStrokePoint2d_size[(sizeof(CStrokePoint2d) == 0xE4) ? 1 : -1];

#endif // CLASSES_CSTROKEPOINT2D_H
