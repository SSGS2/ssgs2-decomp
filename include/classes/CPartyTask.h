#ifndef CLASSES_CPARTYTASK_H
#define CLASSES_CPARTYTASK_H

#include "game/types.h"

// Only the three interface vtable pointers are named so far; the rest of the object is not modelled yet.
class CPartyTask {
public:
    CPartyTask();
    ~CPartyTask();

    static void* GetRTTI();

    void CreateActors();

    // Destructor entry of the tertiary interface, which is 8 bytes past the object's start.
    void* DestroyTertiaryInterface(int deleteFlag);

    void* vtbl;          // 0x00
    void* secondaryVtbl; // 0x04
    void* tertiaryVtbl;  // 0x08
};

#endif // CLASSES_CPARTYTASK_H
