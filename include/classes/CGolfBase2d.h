// CGolfBase2d: the golf HUD that is drawn over the round.
//
// A layout of numbered elements with a power meter, a row of terrain markers
// and a pair of action buttons. The class caches one flag or value per group of
// elements and pushes them into the layout when it is dirty.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so the vtable pointers are members.
// The interface slot names follow from what each slot does.
#ifndef CLASSES_CGOLFBASE2D_H
#define CLASSES_CGOLFBASE2D_H

#include "game/types.h"

class CGolfBase2d {
public:
    // A marker's vertical offset in the marker row and its packed colour.
    struct MarkerStyle {
        float offsetY;
        int color;
    };

    // Construction, the layout and drawing.
    CGolfBase2d* Init();
    CGolfBase2d* Destroy(int flag);
    int LoadLayout(int resourceIndex);
    int InitializeLayoutGeometry();
    int Update();
    int Draw(void* hudObject);
    int RebuildDirtyLayout();

    // Display mode and marker selection.
    int SetDisplayMode(int mode);
    int GetDisplayMode();
    int SetMarkersEnabled(int enabled);
    int SetMarkerSelection(int index, int count);
    int SetMarkerStyles(void* styles);

    // Extra meter image and the action buttons.
    int SetShowExtraMeterImage(int show);
    int SetConfirmButtonEnabled(int enabled);
    int SetActionButtonsEnabled(int enabled);
    int SetActionButtonsVisible(int visible);
    int IsConfirmButtonActivated();
    int IsCancelButtonActivated();
    int ActivateConfirmButton();
    int ActivateCancelButton();

    // Meter values.
    int SetTintColor(int color);
    int SetDisplayValue(float value);
    int StartValueTransition(float value);
    int StopValueTransition();
    int SetPangyaTargetX(float x);
    int SetPangyaZoneWidth(float width);
    int SetMeterOverlayWidth(float width);

    // Dialog fade and element access.
    int StartFadeIn();
    int StartFadeOut();
    int IsFadeSettled();
    int IsFadeVisible();
    int ShowImmediately();
    int UpdateFadeAndMeterTransition();
    int ProcessActivatedActionButtons();
    int ScaleElementToX(int index, float x);

    void* vtbl;              // 0x00  layout base object vtable
    char pad_004[0x58];      // 0x04  layout base object
    int showExtraMeterImage; // 0x5C
    int displayMode;         // 0x60
    int markersEnabled;      // 0x64
    int markerIndex;         // 0x68
    int markerCount;         // 0x6C
    // Style 0 is not drawn: the drawing loop reads style n for marker n, 1..count.
    MarkerStyle markerStyles[21];       // 0x70
    int confirmButtonEnabled;           // 0x118
    int actionButtonsEnabled;           // 0x11C
    int actionButtonsVisible;           // 0x120
    int confirmButtonActivated;         // 0x124
    int cancelButtonActivated;          // 0x128
    int tintColor;                      // 0x12C  packed colour
    float displayValue;                 // 0x130
    float pangyaTargetX;                // 0x134
    float pangyaZoneWidth;              // 0x138
    float meterOverlayWidth;            // 0x13C
    float transitionStartValue;         // 0x140
    int valueTransitionFramesRemaining; // 0x144
    int valueTransitionActive;          // 0x148
    int fadeFramesRemaining;            // 0x14C
    int fadeState;                      // 0x150
    float fadeOpacity;                  // 0x154
    int layoutReady;                    // 0x158
    int layoutDirty;                    // 0x15C
    float markerBaseY;                  // 0x160
    void* interfaceVtable;              // 0x164  vtable of the HUD interface
};

#define CGOLFBASE2D_OFFSET(field, offset) \
    typedef char CGolfBase2d_offset_##field[((int)&((CGolfBase2d*)0)->field == (offset)) ? 1 : -1]
CGOLFBASE2D_OFFSET(showExtraMeterImage, 0x5C);
CGOLFBASE2D_OFFSET(markerStyles, 0x70);
CGOLFBASE2D_OFFSET(confirmButtonEnabled, 0x118);
CGOLFBASE2D_OFFSET(tintColor, 0x12C);
CGOLFBASE2D_OFFSET(transitionStartValue, 0x140);
CGOLFBASE2D_OFFSET(valueTransitionFramesRemaining, 0x144);
CGOLFBASE2D_OFFSET(fadeState, 0x150);
CGOLFBASE2D_OFFSET(markerBaseY, 0x160);
CGOLFBASE2D_OFFSET(interfaceVtable, 0x164);
#undef CGOLFBASE2D_OFFSET

typedef char CGolfBase2d_size[(sizeof(CGolfBase2d) == 0x168) ? 1 : -1];
typedef char CGolfBase2d_marker_style_size[(sizeof(CGolfBase2d::MarkerStyle) == 8) ? 1 : -1];

#endif // CLASSES_CGOLFBASE2D_H
