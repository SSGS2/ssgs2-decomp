// CCloud: the translucent dark tint drawn over the course view. The class holds
// no data of its own; its vtable is still taken from the original game data.
#ifndef CLASSES_CCLOUD_H
#define CLASSES_CCLOUD_H

#include "game/types.h"

class CCloud {
public:
    ~CCloud();

    void vfunc_1();
    static void DrawOverlay();
};

#endif // CLASSES_CCLOUD_H
