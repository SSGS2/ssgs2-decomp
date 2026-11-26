#ifndef CLASSES_CDISKERRORDIALOG_H
#define CLASSES_CDISKERRORDIALOG_H

#include "game/types.h"

// The data layout of CDiskErrorDialog has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CDiskErrorDialog {
public:
    int PAL_LoadImageTexture();
    static void PAL_SetImageIndex(void* obj, int value);
    static int PAL_UpdateOpacityTween(void* obj, float dt);
    static void PAL_ReleaseTextureAndResetFade(void* obj);
    static void PAL_DrawImage(const void* obj);
};

#endif // CLASSES_CDISKERRORDIALOG_H
