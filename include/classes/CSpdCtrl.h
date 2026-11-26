#ifndef CLASSES_CSPDCTRL_H
#define CLASSES_CSPDCTRL_H

// CSpdCtrl: owns the sprites and squares built from a loaded layout resource
// and the BP sprite pool they draw from. It can also clone one of its sprites.

// Pointer lists for the controller's sprites and squares. Their destructors
// live in spd_sprite_manager.cpp.
struct SpdListA {
    void** data;
    unsigned int count;
    unsigned int capacity;
    ~SpdListA();
};
struct SpdListB {
    void** data;
    unsigned int count;
    unsigned int capacity;
    ~SpdListB();
};

class CSpdCtrl {
public:
    CSpdCtrl();

    // Destroy both lists and the sprite pool; free the object too when deletingFlag > 0.
    void* Destroy(int deletingFlag);
    // Clone the sprite at index, give the clone a new pool sprite, and register it. Returns its index.
    unsigned int CloneAndRegister(unsigned int index);

    void* vtbl;                 // 0x00
    SpdListA sprites;           // 0x04
    SpdListB squares;           // 0x10
    void* spritePool;           // 0x1C  owned BP sprite-controller pool
    void* context;              // 0x20  texture source handed to new sprites
    char buildCounts[8];        // 0x24  item counts recorded after a build
    unsigned int resourceFlags; // 0x2C
};

#define CSPDCTRL_OFFSET(field, offset) \
    typedef char CSpdCtrl_offset_##field[((int)&((CSpdCtrl*)0)->field == (offset)) ? 1 : -1]
CSPDCTRL_OFFSET(sprites, 0x04);
CSPDCTRL_OFFSET(squares, 0x10);
CSPDCTRL_OFFSET(spritePool, 0x1C);
CSPDCTRL_OFFSET(context, 0x20);
CSPDCTRL_OFFSET(resourceFlags, 0x2C);
#undef CSPDCTRL_OFFSET
typedef char CSpdCtrl_size[(sizeof(CSpdCtrl) == 0x30) ? 1 : -1];

#endif // CLASSES_CSPDCTRL_H
