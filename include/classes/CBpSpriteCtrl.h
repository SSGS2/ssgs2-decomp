// CBpSpriteCtrl: the 2D sprite pool. It owns its CBpSprite objects and the
// pointer array that holds them, and guards both with one OS mutex.
#ifndef CLASSES_CBPSPRITECTRL_H
#define CLASSES_CBPSPRITECTRL_H

#include "game/types.h"

// The pool's pointer array: entries, the count in use, and the allocated capacity.
struct SpriteBuffer {
    void** entries;
    int count, capacity;
    // Initialize the vector allocation; its enclosing constructor clears size/capacity.
    SpriteBuffer() : entries(0) {}
};

class CBpSpriteCtrl {
public:
    CBpSpriteCtrl();
    ~CBpSpriteCtrl();

    void* vtbl;          // 0x00
    SpriteBuffer buffer; // 0x04
    char mutex[0x10];    // 0x10  OSMutex guarding the pool
};

typedef char CBpSpriteCtrl_size[(sizeof(CBpSpriteCtrl) == 0x20) ? 1 : -1];

#endif // CLASSES_CBPSPRITECTRL_H
