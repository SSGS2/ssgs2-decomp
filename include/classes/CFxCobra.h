#ifndef CLASSES_CFXCOBRA_H
#define CLASSES_CFXCOBRA_H

#include "game/types.h"

struct CFxCobra : CFxControl {
    void* owner;
    int value;
    void* children[4];
    unsigned char flag;
    CFxCobra(void* o, int v);

    int Update();

    // Unregister and destroy the four sequences; delete the control when shouldDelete > 0.
    CFxCobra* Destroy(int shouldDelete);
};

#endif // CLASSES_CFXCOBRA_H
