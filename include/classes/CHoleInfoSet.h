// CHoleInfoSet: the group of hole-information HUD panels.
//
// One set owns the cup/distance, hole, accuracy and timeout panels. The tutorial
// stage gates the first two while the last two have their own enable flags.
#ifndef CLASSES_CHOLEINFOSET_H
#define CLASSES_CHOLEINFOSET_H

#include "game/types.h"

class CHoleCupInfo;
class CHoleInfo;
class CAccuracyInfo;
class CTimeOutInfo;

class CHoleInfoSet {
public:
    // Clear the panel slots and create the four panels.
    CHoleInfoSet* Init();
    void Update(float delta);
    void Draw();
    ~CHoleInfoSet();

    // Create the panels and set the initial enable flags.
    void CreatePanels();

    CHoleCupInfo* holeCupInfo;     // 0x00
    unsigned int reserved04;       // 0x04
    CHoleInfo* holeInfo;           // 0x08
    CAccuracyInfo* accuracyInfo;   // 0x0C
    CTimeOutInfo* timeOutInfo;     // 0x10
    unsigned char holeCupEnabled;  // 0x14
    unsigned char reserved15[2];   // 0x15
    unsigned char accuracyEnabled; // 0x17
    unsigned char timeOutEnabled;  // 0x18
    char pad_019[3];               // 0x19
};

#define CHOLEINFOSET_OFF(f, o) typedef char CHoleInfoSet_off_##f[((int)&((CHoleInfoSet*)0)->f == (o)) ? 1 : -1]
CHOLEINFOSET_OFF(holeCupInfo, 0x00);
CHOLEINFOSET_OFF(holeInfo, 0x08);
CHOLEINFOSET_OFF(accuracyInfo, 0x0C);
CHOLEINFOSET_OFF(timeOutInfo, 0x10);
CHOLEINFOSET_OFF(holeCupEnabled, 0x14);
CHOLEINFOSET_OFF(accuracyEnabled, 0x17);
CHOLEINFOSET_OFF(timeOutEnabled, 0x18);
#undef CHOLEINFOSET_OFF
typedef char CHoleInfoSet_size[(sizeof(CHoleInfoSet) == 0x1C) ? 1 : -1];

#endif // CLASSES_CHOLEINFOSET_H
