#ifndef CLASSES_CDUMMY_H
#define CLASSES_CDUMMY_H

#include "game/types.h"

// A placeholder actor: it ignores every message, draws nothing and has no state
// of its own beyond the vtable the factory installs.
class CDummy {
public:
    void HandleMsg(void* message);
    void** GetRTTI();

    ~CDummy();
};

#endif // CLASSES_CDUMMY_H
