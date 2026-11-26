#ifndef CLASSES_CMAPCAMERA2D_H
#define CLASSES_CMAPCAMERA2D_H

#include "game/types.h"

// The data layout of CMapCamera2d has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CMapCamera2d {
public:
    void* GetFirstActionLatch();
    void* GetSecondActionRequest();
    int RequestSecondActionFeedback();
    int DrawWithOpacity();
    int IsVisibleOrForced();
    int RequestModeIndicators1And4();
    int SetHighlightedMode(void* value);
    int RequestControlsDisplay();
    int SetForceVisible(int value);
    int EnableInputForFrame();
    int StartClosingTransition();
    int IsTransitionIdle();
    int IsFullyOpen();
    int StartOpeningTransition();
    static int LoadLayout(void* layout, int arg);
    int LatchFirstActionAndPlayClick();
    int AlignActionHitAreas();
    int UpdateFadeAndVisibility();
    static int IsEitherActionActivated(void* layout);
    int UpdateActionInput();
    int ApplyPanelLayout();
    int UpdateFrame();

    ~CMapCamera2d();
    CMapCamera2d();
};

#endif // CLASSES_CMAPCAMERA2D_H
