// CRandomHouse: the course's random-house helper.
//
// The base publishes the object as the random-house singleton; the derived part
// owns the MT19937 state that picks the random house. The class is declared
// without virtual functions on purpose: its vtable is still taken from the
// original game data.
#ifndef CLASSES_CRANDOMHOUSE_H
#define CLASSES_CRANDOMHOUSE_H

#include "game/types.h"

extern "C" {
extern int g_randomHouse;
extern char s_randomHouseBaseVtable[16];
extern void BaseObject_ctor(void*);
}

// The base object CRandomHouse extends. Its constructor publishes the object as
// the random-house singleton.
struct RandomHouseBase {
    void* vtbl;        // 0x00
    char pad_004[0xC]; // 0x04

    RandomHouseBase() {
        BaseObject_ctor(this);
        *(void**)this = s_randomHouseBaseVtable;
        g_randomHouse = (int)this;
    }
    ~RandomHouseBase();
};

class CRandomHouse : public RandomHouseBase {
public:
    CRandomHouse();
    ~CRandomHouse();

    unsigned int* data; // 0x10: 624-word MT19937 state
    int nextWordIndex;  // 0x14
    unsigned int seed;  // 0x18
};

#define CRANDOMHOUSE_OFFSET(field, offset) \
    typedef char CRandomHouse_offset_##field[((int)&((CRandomHouse*)0)->field == (offset)) ? 1 : -1]
CRANDOMHOUSE_OFFSET(data, 0x10);
CRANDOMHOUSE_OFFSET(nextWordIndex, 0x14);
CRANDOMHOUSE_OFFSET(seed, 0x18);
#undef CRANDOMHOUSE_OFFSET
typedef char CRandomHouse_size[(sizeof(CRandomHouse) == 0x1C) ? 1 : -1];

#endif // CLASSES_CRANDOMHOUSE_H
