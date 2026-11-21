// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CITEMMANAGER_H
#define CLASSES_CITEMMANAGER_H

#include "game/types.h"
#include "iff/IffStruct.h"

class CItemManager {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char inventoryMutexStorage[0x18];                 // 0x4
    unsigned char lockEnabled;          // 0x1C: guard table access with the mutex at +4
    char unk_1D[3];

    IffTable<IFF_STRUCT::sChar> characters;   // 0x20
    IffTable<IFF_STRUCT::sPart> parts;        // 0x2C
    IffTable<IFF_STRUCT::sClub> clubs;        // 0x38
    IffTable<IFF_STRUCT::sClubSet> clubSets;  // 0x44
    IffTable<IFF_STRUCT::sBall> balls;        // 0x50
    IffTable<IFF_STRUCT::sItem> items;        // 0x5C
    IffTable<IFF_STRUCT::sCaddie> caddies;    // 0x68
    IffTable<IFF_STRUCT::sCadItem> caddieItems; // 0x74
    IffTable<IFF_STRUCT::sSetItem> setItems;  // 0x80
    IffTable<IFF_STRUCT::sCourse> courses;   // 0x8C
    IffTable<IFF_STRUCT::sDesc> descriptions; // 0x98
    IffTable<IFF_STRUCT::sHairStyle> hairStyles; // 0xA4
    IffTable<IFF_STRUCT::sAuxPart> auxParts;  // 0xB0
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CITEMMANAGER_OFF(f, o) typedef char CItemManager_off_##f[((int)&((CItemManager*)0)->f == (o)) ? 1 : -1]
CITEMMANAGER_OFF(lockEnabled, 0x1C);
CITEMMANAGER_OFF(characters, 0x20);
CITEMMANAGER_OFF(parts, 0x2C);
CITEMMANAGER_OFF(clubs, 0x38);
CITEMMANAGER_OFF(clubSets, 0x44);
CITEMMANAGER_OFF(balls, 0x50);
CITEMMANAGER_OFF(items, 0x5C);
CITEMMANAGER_OFF(caddies, 0x68);
CITEMMANAGER_OFF(caddieItems, 0x74);
CITEMMANAGER_OFF(setItems, 0x80);
CITEMMANAGER_OFF(courses, 0x8C);
CITEMMANAGER_OFF(descriptions, 0x98);
CITEMMANAGER_OFF(hairStyles, 0xA4);
CITEMMANAGER_OFF(auxParts, 0xB0);
#undef CITEMMANAGER_OFF
typedef char CItemManager_size_check[(sizeof(CItemManager) == 0xBC) ? 1 : -1];

#endif  // CLASSES_CITEMMANAGER_H
