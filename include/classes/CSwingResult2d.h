#ifndef CLASSES_CSWINGRESULT2D_H
#define CLASSES_CSWINGRESULT2D_H

#include "game/types.h"

// The data layout of CSwingResult2d has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSwingResult2d {
public:
    int vfunc_10();
    int EnableAndRequestRedraw();
    int HideImmediately();
    int StartFadeIn();
    int StartFadeOut();
    int IsVisibleOrFading();
    int SetResultKindAndExtraElement(int resultKind, int showExtraElement);
    static int LoadOwnLayout(void* layout, int loadMode);
    int UpdateFadeCountdown();
    int vfunc_3();
    int RefreshElementColorsAndEnabled();

    CSwingResult2d();
    ~CSwingResult2d();
};

#endif // CLASSES_CSWINGRESULT2D_H
