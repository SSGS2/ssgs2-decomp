// CAccState: the party-item slot manager, seen as the block of modifiers that
// party items enable for the current shot.
//
// Only that block is described here; the manager's item pointers and display
// state belong to the other views of the same object.
#ifndef CLASSES_CACCSTATE_H
#define CLASSES_CACCSTATE_H

#include "classes/PartyItemModifiers.h"
#include "game/types.h"

class CAccState {
public:
    PartyItemModifiers* GetEquipmentBonuses();

    char pad_000[0x10];           // 0x00
    PartyItemModifiers modifiers; // 0x10
};

#define CACCSTATE_OFF(f, o) typedef char CAccState_off_##f[((int)&((CAccState*)0)->f == (o)) ? 1 : -1]
CACCSTATE_OFF(modifiers, 0x10);
#undef CACCSTATE_OFF

#endif // CLASSES_CACCSTATE_H
